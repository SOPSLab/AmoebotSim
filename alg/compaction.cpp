#include <algorithm>  // Used for find() and distance().
#include <vector>

#include "alg/compaction.h"

CompactionParticle::CompactionParticle(const Node head, const int globalTailDir,
                                       const int orientation,
                                       AmoebotSystem &system, State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    followDir(-1) {}

void CompactionParticle::activate() {
  if (isContracted()) {
    if (state == State::Inactive) {
      if (hasNbrInState({State::Seed, State::Active})) {
        state = State::Active;
        followDir = labelOfFirstNbrInState({State::Seed, State::Active});
      }
    } else if (state == State::Active && !hasNbrInState({State::Inactive})) {
      if (!hasHeadAtLabel(followDir)) {
        CompactionParticle& parent = nbrAtLabel(followDir);
        if (parent.isExpanded() && parent.state == State::Follower) {
          // Handover with parent.
          state = State::Follower;
          int parentDir = nbrDirToDir(parent, (parent.tailDir() + 3) % 6);
          push(followDir);
          followDir = parentDir;
          parent.state = State::Active;
        } else if (parent.isExpanded() && parent.state == State::Leader) {
          // Handover with parent and adopt its old parent.
          state = State::Follower;
          int parentDir = nbrDirToDir(parent, (parent.tailDir() + 3) % 6);
          push(followDir);
          followDir = nbrDirToDir(parent, parent.followDir);
          parent.state = State::Active;
          parent.followDir = dirToNbrDir(parent, (parentDir + 3) % 6);
        }
      } else if (hasChild() && !isParticleCompressed() && !hasExpNbr()
                 && !hasNbrInState({State::Follower})) {
        // Expand into the closest unoccupied position to parent.
        state = State::Leader;
        expand(labelOfFirstUnoccupied(followDir));
      } else if (!hasChild() && !hasNbrInState({State::Follower, State::Leader})
                 && numNbrs() != 6) {
        // Perform a leaf switch: choose a different parent at random.
        followDir = randDir();
        while(!hasNbrAtLabel(followDir)
              || (nbrAtLabel(followDir).state != State::Seed
                  && nbrAtLabel(followDir).state != State::Active)) {
          followDir = randDir();
        }
      }
    }
  } else if (isExpanded() && !hasNbrInState({State::Inactive}) && !hasChild()) {
    state = State::Active;
    contractTail();
  }
}

int CompactionParticle::headMarkColor() const {
  switch(state) {
    case State::Seed:     return 0x00ff00;
    case State::Inactive: return -1;
    case State::Active:   return 0x505050;
    case State::Leader:   return 0xff0000;
    case State::Follower: return 0x0000ff;
  }

  return -1;
}

int CompactionParticle::headMarkDir() const {
  return followDir;
}

int CompactionParticle::tailMarkColor() const {
  return headMarkColor();
}

QString CompactionParticle::inspectionText() const {
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
      case State::Active:   return "active\n";
      case State::Leader:   return "leader\n";
      case State::Follower: return "follower\n";
      default:              return "no state\n";
    }
  }();
  text += "  followDir: " + QString::number(followDir) + "\n";
  text += "  #nbrs: " + QString::number(numNbrs()) + "\n";
  text += "  #adj nbrs: ";
  std::vector<int> adjCounts = numAdjNbrs();
  for (int adjCount : adjCounts) {
    text += QString::number(adjCount) + " ";
  }
  text += "\n";

  return text;
}

CompactionParticle& CompactionParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<CompactionParticle>(label);
}

bool CompactionParticle::hasExpNbrTailAtLabel(int label) const {
  if (!hasNbrAtLabel(label)) {
    return false;
  } else {  // hasNbrAtLabel(label).
    // Get labels for the positions one clockwise and one counter-clockwise from
    // the position at label.
    std::vector<int> labels = uniqueLabels();
    int i = distance(labels.begin(), find(labels.begin(), labels.end(), label));
    int cw = labels[(i - 1 + labels.size()) % labels.size()];
    int ccw = labels[(i + 1) % labels.size()];

    CompactionParticle& nbr = nbrAtLabel(label);

    return nbr.isExpanded() && nbr.pointsAtMyTail(*this, label) &&
           (nbr.pointsAtMyHead(*this, cw) || nbr.pointsAtMyHead(*this, ccw));
  }
}

bool CompactionParticle::hasExpNbr() const {
  for (int label : uniqueLabels()) {
    if (hasNbrAtLabel(label) && nbrAtLabel(label).isExpanded()) {
      return true;
    }
  }

  return false;
}

int CompactionParticle::labelOfFirstNbrInState(
    std::initializer_list<State> states, int startLabel) const {
  auto prop = [&](const CompactionParticle& p) {
    for (auto state : states) {
      if (p.state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<CompactionParticle>(prop, startLabel);
}

int CompactionParticle::labelOfFirstUnoccupied(int startLabel) const {
  Q_ASSERT(isContracted());

  for (int offset = 0; offset <= 3; ++offset) {
    int left = (startLabel + offset) % 6;
    int right = (startLabel - offset + 6) % 6;
    if (!hasNbrAtLabel(left)) {
      return left;
    } else if (!hasNbrAtLabel(right)) {
      return right;
    }
  }

  Q_ASSERT(false);
  return -1;
}

bool CompactionParticle::hasNbrInState(std::initializer_list<State> states)
  const {
  return labelOfFirstNbrInState(states) != -1;
}

bool CompactionParticle::hasChild() const {
  auto prop = [&](const CompactionParticle& p) {
    int dir = -1;
    if (p.state == State::Active || p.state == State::Follower) {
      dir = p.dirToHeadLabel(p.followDir);
    } else if (p.state == State::Leader) {
      dir = p.dirToTailLabel(p.followDir);
    } else {
      return false;
    }

    return (isContracted() && pointsAtMe(p, dir)) ||
           (isExpanded() && pointsAtMyTail(p, dir));
  };

  return labelOfFirstNbrWithProperty<CompactionParticle>(prop) != -1;
}

int CompactionParticle::numNbrs() const {
  int numUniqueNbrs = 0;
  for (int label : uniqueLabels()) {
    if (hasNbrAtLabel(label) && !hasExpNbrTailAtLabel(label)) {
      ++numUniqueNbrs;
    }
  }

  return numUniqueNbrs;
}

std::vector<int> CompactionParticle::numAdjNbrs() const {
  // Find first position occupied by a particle with the position immediately
  // clockwise unoccupied or occupied by an expanded tail of a particle fully
  // in this neighborhood.
  std::vector<int> labels = uniqueLabels();
  int start = -1;
  for (uint i = 0; i < labels.size(); ++i) {
    int cw = labels[(i - 1 + labels.size()) % labels.size()];
    if (hasNbrAtLabel(labels[i])
        && (!hasNbrAtLabel(cw) || hasExpNbrTailAtLabel(cw))) {
      start = i;
      break;
    }
  }

  std::vector<int> adjNbrCounts;
  if (start == -1) {
    // If there was no such first position, then either this particle has no
    // neighbors or is surrounded.
    adjNbrCounts.push_back(numNbrs());
  } else {
    int groupCount = 0;
    for (uint offset = 0; offset < labels.size(); ++offset) {
      int label = labels[(start + offset) % labels.size()];
      int cw = labels[(start + offset - 1 + labels.size()) % labels.size()];
      if (hasNbrAtLabel(label) && !hasExpNbrTailAtLabel(label)) {
        ++groupCount;
      } else if ((!hasNbrAtLabel(label) || hasExpNbrTailAtLabel(label))
                 && hasNbrAtLabel(cw)) {
        adjNbrCounts.push_back(groupCount);
        groupCount = 0;
      }
    }
  }

  return adjNbrCounts;
}

bool CompactionParticle::isParticleCompressed() const {
  return isContracted() && numNbrs() != 5 && numAdjNbrs().size() == 1;
}


CompactionSystem::CompactionSystem(int numParticles, float holeProb) {
  Q_ASSERT(numParticles > 0);
  Q_ASSERT(0 <= holeProb && holeProb <= 1);

  // Insert the seed at (0,0).
  insert(new CompactionParticle(Node(0, 0), -1, randDir(), *this,
                                CompactionParticle::State::Seed));
  std::set<Node> occupied;
  occupied.insert(Node(0, 0));

  std::set<Node> candidates;
  for (int i = 0; i < 6; ++i) {
    candidates.insert(Node(0, 0).nodeInDir(i));
  }

  // Add inactive particles.
  int numNonStaticParticles = 1;
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
      insert(new CompactionParticle(randomCandidate, -1, randDir(), *this,
                                    CompactionParticle::State::Inactive));
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

bool CompactionSystem::hasTerminated() const {
  #ifdef QT_DEBUG
    if (!isConnected(particles)) {
      return true;
    }
  #endif

  return false;
}
