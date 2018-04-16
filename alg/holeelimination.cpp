#include "alg/holeelimination.h"

HoleEliminationParticle::HoleEliminationParticle(const Node head,
                                                 const int globalTailDir,
                                                 const int orientation,
                                                 AmoebotSystem& system,
                                                 State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    axisDir(-1),
    moveDir(-1) {}

void HoleEliminationParticle::activate() {
  if (isContracted()) {
    if (state == State::Inactive) {
      if (hasNbrInState({State::Seed, State::Finished})) {
        state = State::Walking;
        updateMoveDir();
      } else if (hasNbrInState({State::Walking, State::Follower})) {
        state = State::Follower;
        moveDir = labelOfFirstNbrInState({State::Walking, State::Follower});
      }
    } else if (state == State::Follower) {
      if (hasNbrInState({State::Finished})) {
        state = State::Walking;
        moveDir = labelOfFirstNbrInState({State::Finished});
      } else if (hasTailAtLabel(moveDir)) {
        auto nbr = nbrAtLabel(moveDir);
        int nbrContractDir = nbrDirToDir(nbr, (nbr.tailDir() + 3) % 6);
        push(moveDir);
        moveDir = nbrContractDir;
      }
    } else if (state == State::Walking) {
      if (hasNbrInState({State::Seed})) {
        state = State::Finished;
        axisDir = (labelOfFirstNbrInState({State::Seed}) + 3) % 6;
      } else if (axisReceiveDir() != -1) {
        state = State::Finished;
        axisDir = (axisReceiveDir() + 3) % 6;
      } else if (adjFinishedNbrsDir() != -1) {
        state = State::Finished;
        moveDir = adjFinishedNbrsDir();
      } else {  // Can't finish yet; continue walking CCW around the surface.
        updateMoveDir();
        if (!hasNbrAtLabel(moveDir)) {
          expand(moveDir);
        } else if (hasTailAtLabel(moveDir)) {
          push(moveDir);
        }
      }
    }
  } else if (isExpanded() && !hasNbrInState({State::Inactive}) &&
             labelOfFirstChild() == -1) {
    contractTail();
  }
}

int HoleEliminationParticle::headMarkColor() const {
  switch(state) {
    case State::Seed:     return 0x00ff00;
    case State::Inactive: return -1;
    case State::Walking:  return 0xff0000;
    case State::Follower: return 0x0000ff;
    case State::Finished: return 0x000000;
  }

  return -1;
}

int HoleEliminationParticle::headMarkDir() const {
  return (axisDir != -1) ? (axisDir + 3) % 6 : moveDir;
}

int HoleEliminationParticle::tailMarkColor() const {
  return headMarkColor();
}

QString HoleEliminationParticle::inspectionText() const {
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
                      + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Local Info:\n";
  text += "  state: ";
  text += [this](){
    switch(state) {
    case State::Seed:     return "seed\n";
    case State::Inactive: return "inactive\n";
    case State::Walking:  return "walking\n";
    case State::Follower: return "follower\n";
    case State::Finished: return "finished\n";
    default:              return "no state\n";
    }
  }();
  text += "  axisDir: " + QString::number(axisDir) + "\n";
  text += "  moveDir: " + QString::number(moveDir) + "\n";

  return text;
}

HoleEliminationParticle& HoleEliminationParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<HoleEliminationParticle>(label);
}

int HoleEliminationParticle::labelOfFirstNbrInState(
    std::initializer_list<State> states, int startLabel) const {
  auto prop = [&](const HoleEliminationParticle& p) {
    for (auto state : states) {
      if (p.state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<HoleEliminationParticle>(prop, startLabel);
}

bool HoleEliminationParticle::hasNbrInState(std::initializer_list<State> states)
    const {
  return labelOfFirstNbrInState(states) != -1;
}

int HoleEliminationParticle::labelOfFirstChild(int startLabel) const {
  auto prop = [&](const HoleEliminationParticle& p) {
    return (p.state == State::Follower || p.state == State::Walking) &&
           pointsAtMyTail(p, p.dirToHeadLabel(p.moveDir));
  };

  return labelOfFirstNbrWithProperty<HoleEliminationParticle>(prop, startLabel);
}

void HoleEliminationParticle::updateMoveDir() {
  moveDir = labelOfFirstNbrInState({State::Seed, State::Finished});
  while (hasNbrAtLabel(moveDir) &&
         (nbrAtLabel(moveDir).state == State::Seed ||
          nbrAtLabel(moveDir).state == State::Finished)) {
    moveDir = (moveDir + 1) % 6;
  }
}

int HoleEliminationParticle::axisReceiveDir() const {
  auto prop = [&](const HoleEliminationParticle& p) {
    return p.isContracted() && p.state == State::Finished &&
           p.axisDir != -1 && pointsAtMe(p, p.axisDir);
  };

  return labelOfFirstNbrWithProperty<HoleEliminationParticle>(prop);
}

int HoleEliminationParticle::adjFinishedNbrsDir() const {
  for (int dir = 0; dir < 6; dir++) {
    if (hasNbrAtLabel(dir) && nbrAtLabel(dir).state == State::Finished) {
      int left = (dir + 1) % 6;
      int right = (dir + 5) % 6;
      if (hasNbrAtLabel(left) && nbrAtLabel(left).state == State::Finished &&
          hasNbrAtLabel(right) && nbrAtLabel(right).state == State::Finished) {
        return dir;
      }
    }
  }

  return -1;
}


HoleEliminationSystem::HoleEliminationSystem(uint numParticles, float holeProb) {
  Q_ASSERT(numParticles > 0);
  Q_ASSERT(0 <= holeProb && holeProb <= 1);

  // Insert the seed at (0,0).
  insert(new HoleEliminationParticle(Node(0, 0), -1, randDir(), *this,
                                     HoleEliminationParticle::State::Seed));
  std::set<Node> occupied;
  occupied.insert(Node(0, 0));

  std::set<Node> candidates;
  for (int i = 0; i < 6; ++i) {
    candidates.insert(Node(0, 0).nodeInDir(i));
  }

  // Add inactive particles.
  uint numNonStaticParticles = 0;
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
      insert(new HoleEliminationParticle(randomCandidate, -1, randDir(), *this,
                                         HoleEliminationParticle::State::Inactive));
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

bool HoleEliminationSystem::hasTerminated() const {
  #ifdef QT_DEBUG
    if (!isConnected(particles)) {
      return true;
    }
  #endif

  for (auto p : particles) {
    auto hp = dynamic_cast<HoleEliminationParticle*>(p);
    if (hp->state != HoleEliminationParticle::State::Seed &&
        hp->state != HoleEliminationParticle::State::Finished) {
      return false;
    }
  }

  return true;
}

