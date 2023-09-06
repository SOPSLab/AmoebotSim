/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/hexagonformation.h"

HexagonFormationParticle::HexagonFormationParticle(const Node head,
                                                   AmoebotSystem& system,
                                                   const State state)
    : AmoebotParticle(head, -1, randDir(), system),
      _state(state),
      _parentDir(-1),
      _hexagonDir(state == State::Seed ? 0 : -1) {}

void HexagonFormationParticle::activate() {
  // alpha_1: idle or follower particles with a seed or retired neighbor become
  // roots and begin traversing the hexagon's surface.
  if (isContracted()
      && (_state == State::Idle || _state == State::Follower)
      && hasNbrInState({State::Seed, State::Retired})) {
    _parentDir = -1;
    _state = State::Root;
    _hexagonDir = nextHexagonDir(1);  // clockwise.
  }
  // alpha_2: idle particles with follower or root neighbors become followers
  // and join the spanning forest.
  else if (_state == State::Idle
           && hasNbrInState({State::Follower, State::Root})) {
    _parentDir = labelOfFirstNbrInState({State::Follower, State::Root});
    _state = State::Follower;
  }
  // alpha_3: contracted roots with no idle neighbors who are pointed at by a
  // retired or seed particle's construction direction retire.
  else if (isContracted()
           && _state == State::Root
           && !hasNbrInState({State::Idle})
           && canRetire()) {
    _hexagonDir = nextHexagonDir(-1);  // counter-clockwise.
    _state = State::Retired;
  }
  // alpha_4: contracted roots that can expand along the surface of the hexagon
  // do so.
  else if (isContracted()
           && _state == State::Root
           && !hasNbrAtLabel(_hexagonDir)) {
    expand(_hexagonDir);
  }
  // alpha_5: expanded followers and roots without idle neighbors but with a
  // tail child pull a tail child in a handover.
  else if (isExpanded()
           && (_state == State::Follower || _state == State::Root)
           && !hasNbrInState({State::Idle})
           && !conTailChildLabels().empty()) {
    if (_state == State::Root)
      _hexagonDir = nextHexagonDir(1);  // clockwise.
    int childLabel = conTailChildLabels()[0];
    nbrAtLabel(childLabel)._parentDir = dirToNbrDir(nbrAtLabel(childLabel),
                                                    (tailDir() + 3) % 6);
    pull(childLabel);
  }
  // alpha_6: expanded followers and roots without idle neighbors or tail
  // children contract their tails.
  else if (isExpanded()
           && (_state == State::Follower || _state == State::Root)
           && !hasNbrInState({State::Idle})
           && !hasTailChild()) {
    if (_state == State::Root)
      _hexagonDir = nextHexagonDir(1);  // clockwise.
    contractTail();
  }
}

int HexagonFormationParticle::headMarkColor() const {
  switch(_state) {
    case State::Seed:      return 0x00ff00;
    case State::Idle:      return -1;
    case State::Follower:  return 0x0000ff;
    case State::Root:      return 0xff0000;
    case State::Retired:   return 0x000000;
    default:               return -1;
  }
}

int HexagonFormationParticle::tailMarkColor() const {
  return headMarkColor();
}

int HexagonFormationParticle::headMarkDir() const {
  if (_state == State::Idle) {
    return -1;
  } else if (_state == State::Follower) {
    return _parentDir;
  } else {  // State::Seed, State::Root, State::Retired.
    return _hexagonDir;
  }
}

QString HexagonFormationParticle::inspectionText() const {
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
          + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Local Info:\n";
  text += "  state: ";
  text += [this](){
    switch(_state) {
      case State::Seed:      return "seed\n";
      case State::Idle:      return "idle\n";
      case State::Follower:  return "follower\n";
      case State::Root:      return "root\n";
      case State::Retired:   return "retired\n";
      default:               return "no state\n";
    }
  }();
  text += "  parentDir: " + QString::number(_parentDir) + "\n";
  text += "  hexagonDir: " + QString::number(_hexagonDir) + "\n";

  return text;
}

HexagonFormationParticle& HexagonFormationParticle::nbrAtLabel(int label) const{
  return AmoebotParticle::nbrAtLabel<HexagonFormationParticle>(label);
}

int HexagonFormationParticle::labelOfFirstNbrInState(
    std::initializer_list<State> states, int startLabel) const {
  auto prop = [&](const HexagonFormationParticle& p) {
    for (auto state : states) {
      if (p._state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<HexagonFormationParticle>(prop, startLabel);
}

bool HexagonFormationParticle::hasNbrInState(
    std::initializer_list<State> states) const {
  return labelOfFirstNbrInState(states) != -1;
}

int HexagonFormationParticle::nextHexagonDir(int orientation) const {
  // First, find a head label that points to a seed or retired neighbor.
  int hexagonLabel;
  for (int label : headLabels()) {
    if (hasNbrAtLabel(label)
        && (nbrAtLabel(label)._state == State::Seed
            || nbrAtLabel(label)._state == State::Retired)) {
      hexagonLabel = label;
      break;
    }
  }

  // Next, find the label that points along the hexagon's surface in a traversal
  // with the specified orientation. Perhaps counterintuitively, this means that
  // we search from the above label in the opposite orientation for the first
  // unoccupied or non-seed/retired neighbor.
  int numLabels = isContracted() ? 6 : 10;
  while (hasNbrAtLabel(hexagonLabel)
         && (nbrAtLabel(hexagonLabel)._state == State::Seed
             || nbrAtLabel(hexagonLabel)._state == State::Retired))
    hexagonLabel = (hexagonLabel + orientation + numLabels) % numLabels;

  // Convert this label to a direction before returning.
  return labelToDir(hexagonLabel);
}

bool HexagonFormationParticle::canRetire() const {
  auto prop = [&](const HexagonFormationParticle& p) {
    return (p._state == State::Seed || p._state == State::Retired)
           && pointsAtMe(p, p._hexagonDir);
  };

  return labelOfFirstNbrWithProperty<HexagonFormationParticle>(prop) != -1;
}

bool HexagonFormationParticle::hasTailChild() const {
  auto prop = [&](const HexagonFormationParticle& p) {
    return p._parentDir != -1
           && pointsAtMyTail(p, p.dirToHeadLabel(p._parentDir));
  };

  return labelOfFirstNbrWithProperty<HexagonFormationParticle>(prop) != -1;
}

const std::vector<int> HexagonFormationParticle::conTailChildLabels() const {
  std::vector<int> labels;
  for (int label : tailLabels())
    if (hasNbrAtLabel(label)
        && nbrAtLabel(label).isContracted()
        && nbrAtLabel(label)._parentDir != -1
        && pointsAtMyTail(nbrAtLabel(label), nbrAtLabel(label)._parentDir))
      labels.push_back(label);

  return labels;
}

HexagonFormationSystem::HexagonFormationSystem(int numParticles,
                                               double holeProb) {
  // Insert the shape formation seed at (0,0).
  std::set<Node> occupied;
  insert(new HexagonFormationParticle(Node(0, 0), *this,
                                      HexagonFormationParticle::State::Seed));
  occupied.insert(Node(0, 0));

  // Initialize the candidate positions set.
  std::set<Node> candidates;
  for (int i = 0; i < 6; ++i) {
    candidates.insert(Node(0, 0).nodeInDir(i));
  }

  // Add all other particles using the random tree algorithm.
  int particlesAdded = 1;
  while (particlesAdded < numParticles && !candidates.empty()) {
    // Pick a random candidate node.
    int randIndex = randInt(0, candidates.size());
    Node randCand;
    for (auto cand = candidates.begin(); cand != candidates.end(); ++cand) {
      if (randIndex == 0) {
        randCand = *cand;
        candidates.erase(cand);
        break;
      } else {
        randIndex--;
      }
    }

    // With probability 1 - holeProb, add a new particle at the candidate node.
    if (randBool(1.0 - holeProb)) {
      insert(new HexagonFormationParticle(
          randCand, *this, HexagonFormationParticle::State::Idle));
      occupied.insert(randCand);
      particlesAdded++;

      // Add new candidates.
      for (int i = 0; i < 6; ++i) {
        if (occupied.find(randCand.nodeInDir(i)) == occupied.end()) {
          candidates.insert(randCand.nodeInDir(i));
        }
      }
    }
  }
}

bool HexagonFormationSystem::hasTerminated() const {
  for (auto p : particles) {
    auto hp = dynamic_cast<HexagonFormationParticle*>(p);
    if (hp->_state != HexagonFormationParticle::State::Seed
        && hp->_state != HexagonFormationParticle::State::Retired)
      return false;
  }

  return true;
}
