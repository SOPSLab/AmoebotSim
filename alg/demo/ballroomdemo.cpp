/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/demo/ballroomdemo.h"

#include <iostream>

BallroomDemoParticle::BallroomDemoParticle(const Node head, const int globalTailDir,
                                   const int orientation, AmoebotSystem &system,
                                   State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state) {}

void BallroomDemoParticle::activate() {
  if (isContracted()) {
    if (state == State::Leader) {
      int nbrLabel = labelOfNeighbor(&*neighbor);
      bool flwrExists = (nbrLabel != -1) && (neighbor->state == State::Follower);
      if (flwrExists) {
        if (canPush(nbrLabel)) {
          push(nbrLabel);
          (*neighbor).state = State::Leader;
          state = State::Follower;
        } else { // Choose a random move direction not occupied by the follower.
          int moveDir = randDir();
          if (canExpand(moveDir))
            expand(moveDir);
        }
      }
    }
  }
  else {  // isExpanded().
    if (state == State::Leader) {
      int nbrLabel = labelOfNeighbor(&*neighbor);
      bool flwrExists = (nbrLabel != -1) && (neighbor->state == State::Follower);
      if (flwrExists) {
        if (canPull(nbrLabel)) {
          pull(nbrLabel);
          (*neighbor).contractTail();
          (*neighbor).state = State::Leader;
          state = State::Follower;
        }
      }
    }
  }
}

int BallroomDemoParticle::headMarkColor() const {
  switch(state) {
    case State::Leader:   return 0xff0000;
    case State::Follower: return 0x0000ff;
  }

  return -1;
}

int BallroomDemoParticle::tailMarkColor() const {
  return headMarkColor();
}

int BallroomDemoParticle::labelOfNeighbor(BallroomDemoParticle *neighbor) const {
  const int maxNeighbors = isExpanded() ? 12 : 6;
  for(int i = 0; i < maxNeighbors; ++i) {
    bool hasnbr = hasNbrAtLabel(i);
    if (hasnbr) {
      if(&nbrAtLabel(i) == &*neighbor) {
        return i;
      }
    }
  }
  return -1;
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
    switch(state) {
      case State::Leader:   return "leader\n";
      case State::Follower: return "follower\n";
      default:              return "no state\n";
    }
  }();

  return text;
}

BallroomDemoParticle& BallroomDemoParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<BallroomDemoParticle>(label);
}

BallroomDemoSystem::BallroomDemoSystem(unsigned int numParticles) {
  // Insert the leader at (0,0) and the follower at (-1,0).
  std::set<Node> occupied;
  while (occupied.size() < numParticles) {
    // First, choose an x and y position at random from the (i) and (ii) bounds.
    int x = randInt(-numParticles, numParticles);
    int y = randInt(-numParticles, numParticles);
    Node node(x, y);
    int partnerDir = randDir();
    Node flwr = node.nodeInDir(partnerDir);
    int leaderExpandDir = randDir();
    while (leaderExpandDir == partnerDir) {
      leaderExpandDir = randDir();
    }

    // If unoccupied, place a particle there.
    if (occupied.find(node) == occupied.end() &&
        occupied.find(flwr) == occupied.end()) {
      BallroomDemoParticle *one = new BallroomDemoParticle(node, leaderExpandDir, randDir(), *this,
                                                  BallroomDemoParticle::State::Leader);
      insert(one);
      BallroomDemoParticle *two = new BallroomDemoParticle(flwr, -1, randDir(), *this,
                                                  BallroomDemoParticle::State::Follower);
      insert(two);
      one->neighbor = &(*two);
      two->neighbor = &(*one);
      occupied.insert(node);
      occupied.insert(flwr);
    }
  }
}
