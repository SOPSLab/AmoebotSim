/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/demo/ballroomdemo.h"

BallroomDemoParticle::BallroomDemoParticle(const Node head,
                                           const int globalTailDir,
                                           const int orientation,
                                           AmoebotSystem &system,
                                           State state)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      _state(state),
      _partnerLbl(-1),
      _timesPushed(0),
      _timesPulled(0) {}

void BallroomDemoParticle::activate() {
  if (_state == State::Leader) {
    if (isContracted()) {
      // Attempt to expand into an random adjacent position.
      int expandDir = randDir();
      if (canExpand(expandDir)) {
        expand(expandDir);
      }
    } else {
      // Find the follower partner and pull it, if possible.
      for (int label : tailLabels()) {
        if (hasNbrAtLabel(label) && nbrAtLabel(label)._partnerLbl != -1
            && pointsAtMe(nbrAtLabel(label), nbrAtLabel(label)._partnerLbl)) {
          if (canPull(label)) {
            nbrAtLabel(label)._partnerLbl =
                dirToNbrDir(nbrAtLabel(label), (tailDir() + 3) % 6);
            nbrAtLabel(label)._timesPulled++;
            pull(label);
          }
          break;
        }
      }
    }
  } else {  // _state == State::Follower.
    if (isContracted()) {
      // Push the leader partner, if possible.
      if (canPush(_partnerLbl)) {
        auto leader = nbrAtLabel(_partnerLbl);
        int leaderContractDir = nbrDirToDir(leader, (leader.tailDir() + 3) % 6);
        nbrAtLabel(_partnerLbl)._timesPushed++;
        push(_partnerLbl);
        _partnerLbl = leaderContractDir;
      }
    } else {
      contractTail();
    }
  }
}

int BallroomDemoParticle::headMarkColor() const {
  switch(_state) {
    case State::Leader:   return 0xff0000;
    case State::Follower: return 0x0000ff;
  }

  return -1;
}

int BallroomDemoParticle::headMarkDir() const {
  return _state == State::Follower ? _partnerLbl : -1;
}

int BallroomDemoParticle::tailMarkColor() const {
  return headMarkColor();
}

QString BallroomDemoParticle::inspectionText() const {
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
      case State::Leader:   return "leader\n";
      case State::Follower: return "follower\n";
    }
    return "no state\n";
  }();
  text += "  partnerDir: " + QString::number(_partnerLbl) + "\n";
  text += "  # times pushed: " + QString::number(_timesPushed) + "\n";
  text += "  # times pulled: " + QString::number(_timesPulled);

  return text;
}

BallroomDemoParticle& BallroomDemoParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<BallroomDemoParticle>(label);
}

BallroomDemoSystem::BallroomDemoSystem(unsigned int numParticles) {
  // To enclose an area that's roughly 6x the # of particles using a rhombus,
  // the rhombus should have side length 2.6*sqrt(# particles).
  int sideLen = static_cast<int>(std::round(2.6 * std::sqrt(numParticles)));
  Node boundNode(0, 0);
  std::vector<int> rhombusDirs = {0, 1, 3, 4};
  for (int dir : rhombusDirs) {
    for (int i = 0; i < sideLen; ++i) {
      insert(new Object(boundNode));
      boundNode = boundNode.nodeInDir(dir);
    }
  }

  // Let s be the bounding rhombus side length. When the rhombus is created as
  // above, the nodes (x,y) strictly within the rhombus have (i) 0 < x < s and
  // (ii) 0 < y < s. We want to instantiate particles in Leader/Follower pairs,
  // or "dance partners".
  std::set<Node> occupied;
  unsigned int numParticlesAdded = 0;
  while (numParticlesAdded < numParticles) {
    // Choose an (x,y) position within the rhombus for the Leader and a random
    // adjacent node for its Follower partner.
    Node leaderNode(randInt(2, sideLen - 1), randInt(2, sideLen - 1));
    int followerDir = randDir();
    Node followerNode = leaderNode.nodeInDir(followerDir);

    // If both nodes are unoccupied, place the pair there, linking them together
    // by setting their partner directions to face one another.
    if (occupied.find(leaderNode) == occupied.end()
        && occupied.find(followerNode) == occupied.end()) {
      BallroomDemoParticle* leader =
          new BallroomDemoParticle(leaderNode, -1, randDir(), *this,
                                   BallroomDemoParticle::State::Leader);
      insert(leader);
      occupied.insert(leaderNode);

      BallroomDemoParticle* follower =
          new BallroomDemoParticle(followerNode, -1, randDir(), *this,
                                   BallroomDemoParticle::State::Follower);
      follower->_partnerLbl = follower->globalToLocalDir((followerDir + 3) % 6);
      insert(follower);
      occupied.insert(followerNode);

      numParticlesAdded += 2;
    }
  }
}
