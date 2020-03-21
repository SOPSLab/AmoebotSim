/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/demo/pulldemo.h"

PullDemoParticle::PullDemoParticle(const Node head, const int globalTailDir,
                                   const int orientation, AmoebotSystem &system,
                                   State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    moveDir(-1) {}

void PullDemoParticle::activate() {
  if (isContracted()) {
    if (state == State::Leader) {
      // Choose a random move direction not occupied by the follower.
      moveDir = randDir();
      while (hasNbrAtLabel(moveDir)) {
        moveDir = randDir();
      }
      expand(moveDir);
    }
  } else {  // isExpanded().
    if (state == State::Leader) {
      // Pull handover with the follower if it is contracted.
      int nbrLabel = labelOfFirstNbrInState({State::Follower});
      PullDemoParticle& nbr = nbrAtLabel(nbrLabel);
      if (nbr.isContracted()) {
        int contractDir = (tailDir() + 3) % 6;
        pull(nbrLabel);
        nbr.moveDir = dirToNbrDir(nbr, contractDir);
      }
    } else if (state == State::Follower) {
      contractTail();
      moveDir = labelOfFirstNbrInState({State::Leader});
    }
  }
}

int PullDemoParticle::headMarkColor() const {
  switch(state) {
    case State::Leader:   return 0xff0000;
    case State::Follower: return 0x0000ff;
  }

  return -1;
}

int PullDemoParticle::headMarkDir() const {
  return moveDir;
}

int PullDemoParticle::tailMarkColor() const {
  return headMarkColor();
}

QString PullDemoParticle::inspectionText() const {
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
  text += "  moveDir: " + QString::number(moveDir) + "\n";

  return text;
}

PullDemoParticle& PullDemoParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<PullDemoParticle>(label);
}

int PullDemoParticle::labelOfFirstNbrInState(
    std::initializer_list<State> states, int startLabel) const {
  auto prop = [&](const PullDemoParticle& p) {
    for (auto state : states) {
      if (p.state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<PullDemoParticle>(prop, startLabel);
}


PullDemoSystem::PullDemoSystem() {
  // Insert the leader at (0,0) and the follower at (-1,0).
  insert(new PullDemoParticle(Node(0, 0), -1, randDir(), *this,
                              PullDemoParticle::State::Leader));
  insert(new PullDemoParticle(Node(-1, 0), -1, randDir(), *this,
                              PullDemoParticle::State::Follower));
}

bool PullDemoSystem::hasTerminated() const {
  #ifdef QT_DEBUG
    if (!isConnected(particles)) {
      return true;
    }
  #endif

  return false;
}
