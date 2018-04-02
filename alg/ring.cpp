#include <QtGlobal>

#include <set>
#include <vector>

#include "alg/ring.h"

RingParticle::RingParticle(const Node head, const int globalTailDir,
                                 const int orientation, AmoebotSystem& system,
                                 State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    constructionDir(-1),
    moveDir(-1),
    followDir(-1) {
  totalNumber = -1;
  moveNum = -1;
  if (state == State::Seed) {
      constructionDir = 0;
      totalNumber = 1;
  }
}

void RingParticle::activate() {
  if (isExpanded()) {
    if (state == State::Follow) {
      if (!hasNbrInState({State::Idle}) && !hasTailFollower()) {
        contractTail();
      }
      return;
    } else if (state == State::Lead) {
      if (!hasNbrInState({State::Idle}) && !hasTailFollower()) {
        contractTail();
        updateMoveDir();
      }
      return;
    } else {
      Q_ASSERT(false);
    }
  } else {
    if (state == State::Seed) {
      if (hasNbrAtLabel(constructionDir) &&
          nbrAtLabel(constructionDir).state == State::Follow &&
          nbrAtLabel(constructionDir).totalNumber > 0) {
        state = State::Follow;
        followDir = constructionDir;
        return;
      }
      return;
    } else if (state == State::Idle) {
      if (hasNbrInState({State::Seed, State::Wait})) {
        state = State::Lead;
        updateMoveDir();
        return;
      } else if (hasNbrInState({State::Lead, State::Follow})) {
        state = State::Follow;
        followDir = labelOfFirstNbrInState({State::Lead, State::Follow});
        return;
      }
    } else if (state == State::Follow) {
      if (hasNbrInState({State::Seed, State::Wait}) && totalNumber < 0) {
        state = State::Lead;
        updateMoveDir();
        return;
      } else if (hasNbrInState({State::Finish}) &&
                 labelOfFirstNbrInState({State::Finish}, followDir)
                 == followDir) {
        state = State::Lead;
        moveDir = followDir;
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
      if (canWait() && totalNumber < 0) {
        moveNum = 4;              // Set arbitrary wait time to 4 passes
        state = State::Wait;
        updateConstructionDir();
        return;
      } else if (canFinish()) {
        state = State::Finish;
        updateConstructionDir();
        return;
      } else if (totalNumber > 0 && hasNbrAtLabel(moveDir)) {
        if (nbrAtLabel(moveDir).state == State::Follow ||  // Set state to
            nbrAtLabel(moveDir).state == State::Finish){   // finish if there is
          state = State::Finish;                           // a follow/finish
          updateConstructionDir();                         // particle at
          return;                                          // moveDir.
        }
      } else {
        updateMoveDir();
        if (state == State::Finish) return;
        if (!hasNbrAtLabel(moveDir)) {
          moveNum++;            // Only count expansions as movements
          expand(moveDir);
        } else if (hasTailAtLabel(moveDir)) {
          push(moveDir);
        }
        return;
      }
    } else if (state == State::Wait) {
      if (nbrAtLabel(constructionWaitReceiveDir()).totalNumber > 0
          && totalNumber < 0) {
        if (moveNum > 0) {      // Use moveNum as arbitrary wait before getting
          moveNum--;            // totalNum from neighbor.
          return;
        }
        int dir = constructionWaitReceiveDir();
        totalNumber = nbrAtLabel(dir).totalNumber + 1;
        return;
      }
      else if (!hasNbrAtLabel(constructionDir) && totalNumber > 0) {
        state = State::Lead;             // Set the last wait node to be the
        moveDir = constructionDir;       // lead particle for ring formation.
        updateMoveDir();
        return;
      }
      else if (totalNumber > 0 && hasNbrInState({State::Lead, State::Follow})) {
        int label = labelOfFirstNbrInState({State::Lead, State::Follow},
                                           constructionDir);
        if (label == constructionDir &&           // Set the waiting particle to
            nbrAtLabel(label).totalNumber > 0) {  // follow if the particle it
          state = State::Follow;                  // is pointing to is in Follow
          followDir = constructionDir;            // state and has totalNumber
          return;                                 // greater than 0.
        }
        return;
      }
      return;
    }
  }
}

int RingParticle::headMarkColor() const {
  switch(state) {
  case State::Seed:   return 0x00ff00;
  case State::Idle:   return -1;
  case State::Follow: return 0x0000ff;
  case State::Lead:   return 0xff0000;
  case State::Finish: return 0x000000;
  case State::Wait:   if (totalNumber < 0) return 0xffff00;
                      else                 return 0xff00ff;
  }

  return -1;
}

int RingParticle::headMarkDir() const {
  if (state == State::Lead) {
    return moveDir;
  } else if (state == State::Seed || state == State::Finish ||
             state == State::Wait) {
    return constructionDir;
  } else if (state == State::Follow) {
    return followDir;
  }
  return -1;
}

int RingParticle::tailMarkColor() const {
  return headMarkColor();
}

QString RingParticle::inspectionText() const {
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
    case State::Wait:   return "wait";
    default:            return "no state";
    }
  }();
  text += "\n";
  text += "followDir: " + QString::number(followDir) + "\n";
  text += "moveDir: " + QString::number(moveDir) + "\n";
  text += "constructionDir: " + QString::number(constructionDir) + "\n";
  text += "totalNumber: " + QString::number(totalNumber) + "\n";
  text += "moveNum: " + QString::number(moveNum) + "\n";

  return text;
}

RingParticle& RingParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<RingParticle>(label);
}

int RingParticle::labelOfFirstNbrInState(std::initializer_list<State> states,
                                            int startLabel) const {
  auto propertyCheck = [&](const RingParticle& p) {
    for (auto state : states) {
      if (p.state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<RingParticle>(propertyCheck,
                                                           startLabel);
}

bool RingParticle::hasNbrInState(std::initializer_list<State> states)
  const {
  return labelOfFirstNbrInState(states) != -1;
}

int RingParticle::constructionFinishReceiveDir() const {
  auto propertyCheck = [&](const RingParticle& p) {
    return isContracted() &&
           (p.state == State::Seed || p.state == State::Finish) &&
           pointsAtMe(p, p.constructionDir);
  };

  return labelOfFirstNbrWithProperty<RingParticle>(propertyCheck);
}

int RingParticle::constructionWaitReceiveDir() const {
  auto propertyCheck = [&](const RingParticle& p) {
    return isContracted() &&
           (p.state == State::Seed || p.state == State::Wait) &&
           pointsAtMe(p, p.constructionDir);
  };

  return labelOfFirstNbrWithProperty<RingParticle>(propertyCheck);
}

bool RingParticle::canFinish() const {
  return constructionFinishReceiveDir() != -1;
}

bool RingParticle::canWait() const {
  return constructionWaitReceiveDir() != -1;
}

void RingParticle::updateConstructionDir() {
  if (state == State::Wait) {
    constructionDir = constructionWaitReceiveDir();
    if (nbrAtLabel(constructionDir).state == State::Seed) {
      constructionDir = (constructionDir + 1) % 6;
    } else {
      constructionDir = (constructionDir + 2) % 6;
    }

    if (hasNbrAtLabel(constructionDir) &&
        nbrAtLabel(constructionDir).state == State::Wait) {
      constructionDir = (constructionDir + 1) % 6;
    }
  } else if(state == State::Finish) {
    constructionDir = (moveDir + 3) % 6;
    if (!hasNbrAtLabel(constructionDir) ||       // Increment constructionDir to
        (hasNbrAtLabel(constructionDir) &&       // account for turning.
         nbrAtLabel(constructionDir).state == State::Finish)) {
      constructionDir = (constructionDir + 1) % 6;
    }
  }
}

void RingParticle::updateMoveDir() {
  if (totalNumber < 0) {
    moveDir = labelOfFirstNbrInState({State::Seed, State::Wait});
    while (hasNbrAtLabel(moveDir) && (nbrAtLabel(moveDir).state == State::Seed
                                      ||
                                      nbrAtLabel(moveDir).state == State::Wait))
    {
      moveDir = (moveDir + 5) % 6;
    }
  }
  else {
    if (!hasNbrAtLabel((moveDir + 5) % 6)) {     // If making a turn, reset
      moveDir = (moveDir + 5) % 6;               // moveNum to 0 so that the
      moveNum = 0;                               // particle does not stop
    }                                            // prematurely.
    if (moveNum == 2 * (totalNumber / 6) / 3     // Determine stop moveNum that
        && !isExpanded()) {                      // will give a layer of at most
      state = State::Finish;                     // 2 particles.
      constructionDir = (moveDir + 3) % 6;
    }
  }
}

bool RingParticle::hasTailFollower() const {
  auto propertyCheck = [&](const RingParticle& p) {
    return p.state == State::Follow &&
           pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
  };

  return labelOfFirstNbrWithProperty<RingParticle>(propertyCheck) != -1;
}

RingSystem::RingSystem(int numParticles, float holeProb) {
  Q_ASSERT(numParticles > 0);
  Q_ASSERT(0 <= holeProb && holeProb <= 1);

  // Insert the seed at (0,0).
  insert(new RingParticle(Node(0, 0), -1, randDir(), *this,
                             RingParticle::State::Seed));
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
      insert(new RingParticle(randomCandidate, -1, randDir(), *this,
                                 RingParticle::State::Idle));
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

bool RingSystem::hasTerminated() const {
  #ifdef QT_DEBUG
    if (!isConnected(particles)) {
      return true;
    }
  #endif

  for (auto p : particles) {
    auto hp = dynamic_cast<RingParticle*>(p);
    if (hp->state != RingParticle::State::Seed &&
        hp->state != RingParticle::State::Finish) {
      return false;
    }
  }

  return true;
}
