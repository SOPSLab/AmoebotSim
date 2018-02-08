#include <QtGlobal>

#include <set>
#include <vector>

#include "alg/hexagon.h"

HexagonParticle::HexagonParticle(const Node head, const int globalTailDir,
                                 const int orientation, AmoebotSystem& system,
                                 State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    constructionDir(-1),
    moveDir(-1),
    followDir(-1) {
  if (state == State::Seed) {
      constructionDir = 0;
  }
}

void HexagonParticle::activate() {
  if (isExpanded()) {
    if (state == State::Follow) {
      if (!hasNeighborInState({State::Idle}) && !hasTailFollower()) {
        contractTail();
      }
      return;
    } else if (state == State::Lead) {
      if (!hasNeighborInState({State::Idle}) && !hasTailFollower()) {
        contractTail();
        updateMoveDir();
      }
      return;
    } else {
      Q_ASSERT(false);
    }
  } else {
    if (state == State::Seed) {
      return;
    } else if (state == State::Idle) {
      if (hasNeighborInState({State::Seed, State::Finish})) {
        state = State::Lead;
        updateMoveDir();
        return;
      } else if (hasNeighborInState({State::Lead, State::Follow})) {
        state = State::Follow;
        followDir = labelOfFirstNbrInState({State::Lead, State::Follow});
        return;
      }
    } else if (state == State::Follow) {
      if (hasNeighborInState({State::Seed, State::Finish})) {
        state = State::Lead;
        updateMoveDir();
        return;
      } else if (hasTailAtLabel(followDir)) {
        auto neighbor = nbrAtLabel(followDir);
        int neighborContractionDir =
          nbrDirToDir(neighbor, (neighbor.tailDir() + 3) % 6);
        push(followDir);
        followDir = neighborContractionDir;
        return;
      }
    } else if (state == State::Lead) {
      if (canFinish()) {
        state = State::Finish;
        updateConstructionDir();
        return;
      } else {
        updateMoveDir();
        if (!hasNbrAtLabel(moveDir)) {
          expand(moveDir);
        } else if (hasTailAtLabel(moveDir)) {
          push(moveDir);
        }
        return;
      }
    }
  }
}

int HexagonParticle::headMarkColor() const {
  switch(state) {
  case State::Seed:   return 0x00ff00;
  case State::Idle:   return -1;
  case State::Follow: return 0x0000ff;
  case State::Lead:   return 0xff0000;
  case State::Finish: return 0x000000;
  }

  return -1;
}

int HexagonParticle::headMarkDir() const {
  if (state == State::Lead) {
    return moveDir;
  } else if (state == State::Seed || state == State::Finish) {
    return constructionDir;
  } else if (state == State::Follow) {
    return followDir;
  }
  return -1;
}

int HexagonParticle::tailMarkColor() const {
  return headMarkColor();
}

QString HexagonParticle::inspectionText() const {
  QString text;
  text += "head: (" + QString::number(head.x) + ", " + QString::number(head.y) +
    ")\n";
  text += "orientation: " + QString::number(orientation) + "\n";
  text += "globalTailDir: " + QString::number(globalTailDir) + "\n";
  text += "state: ";
  text += [this](){
    switch(state) {
    case State::Seed:   return "seed";
    case State::Idle:   return "idle";
    case State::Follow: return "follow";
    case State::Lead:   return "lead";
    case State::Finish: return "finish";
    default:            return "no state";
    }
  }();
  text += "\n";
  text += "followDir: " + QString::number(followDir) + "\n";
  text += "moveDir: " + QString::number(moveDir) + "\n";
  text += "constructionDir: " + QString::number(constructionDir) + "\n";

  return text;
}

HexagonParticle& HexagonParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<HexagonParticle>(label);
}

int HexagonParticle::labelOfFirstNbrInState(std::initializer_list<State> states,
                                            int startLabel) const {
  auto propertyCheck = [&](const HexagonParticle& p) {
    for (auto state : states) {
      if (p.state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<HexagonParticle>(propertyCheck,
                                                           startLabel);
}

bool HexagonParticle::hasNeighborInState(std::initializer_list<State> states)
  const {
  return labelOfFirstNbrInState(states) != -1;
}

int HexagonParticle::constructionReceiveDir() const {
  auto propertyCheck = [&](const HexagonParticle& p) {
    return isContracted() &&
           (p.state == State::Seed || p.state == State::Finish) &&
           pointsAtMe(p, p.constructionDir);
  };

  return labelOfFirstNbrWithProperty<HexagonParticle>(propertyCheck);
}

bool HexagonParticle::canFinish() const {
  return constructionReceiveDir() != -1;
}

void HexagonParticle::updateConstructionDir() {
  constructionDir = constructionReceiveDir();
  if (nbrAtLabel(constructionDir).state == State::Seed) {
    constructionDir = (constructionDir + 1) % 6;
  } else {
    constructionDir = (constructionDir + 2) % 6;
  }

  if(hasNbrAtLabel(constructionDir) &&
     nbrAtLabel(constructionDir).state == State::Finish) {
    constructionDir = (constructionDir + 1) % 6;
  }
}

void HexagonParticle::updateMoveDir() {
  moveDir = labelOfFirstNbrInState({State::Seed, State::Finish});
  while (hasNbrAtLabel(moveDir) && (nbrAtLabel(moveDir).state == State::Seed ||
                                    nbrAtLabel(moveDir).state == State::Finish))
  {
    moveDir = (moveDir + 5) % 6;
  }
}

bool HexagonParticle::hasTailFollower() const {
  auto propertyCheck = [&](const HexagonParticle& p) {
    return p.state == State::Follow &&
           pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
  };

  return labelOfFirstNbrWithProperty<HexagonParticle>(propertyCheck) != -1;
}

HexagonSystem::HexagonSystem(int numParticles, float holeProb) {
  Q_ASSERT(numParticles > 0);
  Q_ASSERT(0 <= holeProb && holeProb <= 1);

  // Insert the seed at (0,0).
  insert(new HexagonParticle(Node(0, 0), -1, randDir(), *this,
                             HexagonParticle::State::Seed));
  std::set<Node> occupied;
  occupied.insert(Node(0, 0));

  std::set<Node> candidates;
  for (int i = 0; i < 6; ++i) {
    candidates.insert(Node(0, 0).nodeInDir(i));
  }

  // Add inactive particles.
  int numNonStaticParticles = 0;
  while (numNonStaticParticles < numParticles && !candidates.empty()) {
    // Pick random candidate.
    int randIndex = randInt(0, candidates.size());
    Node randomCandidate;
    for (auto it = candidates.begin(); it != candidates.end(); ++it) {
      if (randIndex == 0) {
        randomCandidate = *it;
        candidates.erase(it);
        break;
      } else {
        randIndex--;
      }
    }

    occupied.insert(randomCandidate);

    // Add this candidate as a particle if not a hole.
    if (randBool(1.0f - holeProb)) {
      insert(new HexagonParticle(randomCandidate, -1, randDir(), *this,
                                 HexagonParticle::State::Idle));
      ++numNonStaticParticles;

      // Add new candidates.
      for (int i = 0; i < 6; ++i) {
        auto neighbor = randomCandidate.nodeInDir(i);
        if (occupied.find(neighbor) == occupied.end()) {
          candidates.insert(neighbor);
        }
      }
    }
  }
}

bool HexagonSystem::hasTerminated() const {
  #ifdef QT_DEBUG
    if (!isConnected(particles)) {
      return true;
    }
  #endif

  for (auto p : particles) {
    auto hp = dynamic_cast<HexagonParticle*>(p);
    if (hp->state != HexagonParticle::State::Seed &&
        hp->state != HexagonParticle::State::Finish) {
      return false;
    }
  }

  return true;
}
