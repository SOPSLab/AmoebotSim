/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/infobjcoating.h"

#include <set>

InfObjCoatingParticle::InfObjCoatingParticle(const Node head,
                                             const int globalTailDir,
                                             const int orientation,
                                             AmoebotSystem &system, State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    moveDir(-1) {}

void InfObjCoatingParticle::activate() {
  if (isExpanded()) {
    if (state == State::Follower) {
      // If an expanded follower has no child or inactive neighbor, contract.
      if (!hasFollowerChild() && !hasNbrInState({State::Inactive})) {
        contractTail();
        return;
      }
    } else if (state == State::Leader) {
      // If an expanded leader holds a complaint token and has no child or
      // inactive neighbor, contract and consume token.
      if (hasToken<ComplaintToken>() && !hasNbrInState({State::Inactive})
          && !hasFollowerChild()) {
        contractTail();
        takeToken<ComplaintToken>();
        return;
      }
    }
  } else {  // Particle is contracted.
    if (state == State::Inactive) {
      // Inactive particles need to first join the spanning tree.
      if (hasObjectNbr()) {
        state = State::Leader;
        moveDir = nextSurfaceDir();
        return;
      } else if (hasNbrInState({State::Leader, State::Follower})) {
        state = State::Follower;
        moveDir = labelOfFirstNbrInState({State::Leader, State::Follower});
        return;
      }
    } else if (state == State::Follower) {
      if (hasObjectNbr()) {
        // If a follower has followed its spanning tree to the surface, become a
        // leader, removing follow direction and calculating move direction.
        state = State::Leader;
        moveDir = nextSurfaceDir();
        return;
      } else if (hasTailAtLabel(moveDir)) {
        // If a follower's parent is expanded, handover expand with it. Update
        // moveDir to continue to point at the parent after the handover.
        auto nbr = nbrAtLabel(moveDir);
        int nbrContractDir = nbrDirToDir(nbr, (nbr.tailDir() + 3) % 6);
        push(moveDir);
        moveDir = nbrContractDir;
        return;
      }
    } else if (state == State::Leader) {
      // If has a follower child, generate a complaint token if not holding one.
      if (hasFollowerChild() && !hasToken<ComplaintToken>()) {
        putToken(std::make_shared<ComplaintToken>());
      }

      // Only act if holding a complaint token.
      if (hasToken<ComplaintToken>()) {
        moveDir = nextSurfaceDir();
        if (!hasNbrAtLabel(moveDir)) {
          // If there is no particle ahead, expand and consume complaint token.
          expand(moveDir);
          takeToken<ComplaintToken>();
          return;
        } else if (hasTailAtLabel(moveDir)) {
          // If there is an expanded particle ahead, handover expand and consume
          // complaint token.
          push(moveDir);
          takeToken<ComplaintToken>();
          return;
        } else {
          // Cannot expand or handover expand; attempt to forward complaint.
          InfObjCoatingParticle& nbr = nbrAtLabel(moveDir);
          if (nbr.state == State::Leader && !nbr.hasToken<ComplaintToken>()) {
            nbr.putToken(takeToken<ComplaintToken>());
          }
          return;
        }
      }
    }
  }
}

int InfObjCoatingParticle::headMarkColor() const {
  if (hasToken<ComplaintToken>()) {
    return 0xffaa00;
  }

  switch(state) {
    case State::Inactive: return -1;
    case State::Follower: return 0x0000ff;
    case State::Leader:   return 0xff0000;
  }

  return -1;
}

int InfObjCoatingParticle::headMarkDir() const {
  return (state == State::Leader || state == State::Follower) ? moveDir : -1;
}

int InfObjCoatingParticle::tailMarkColor() const {
  return headMarkColor();
}

QString InfObjCoatingParticle::inspectionText() const {
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
    case State::Inactive: return "inactive\n";
    case State::Follower: return "follower\n";
    case State::Leader:   return "leader\n";
    default:              return "no state\n";
    }
  }();
  text += "  moveDir: " + QString::number(moveDir) + "\n";
  text += "  complaint: " + QString::number(hasToken<ComplaintToken>()) + "\n";

  return text;
}

InfObjCoatingParticle& InfObjCoatingParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<InfObjCoatingParticle>(label);
}

int InfObjCoatingParticle::labelOfFirstNbrInState(
    std::initializer_list<State> states, int startLabel) const {
  auto prop = [&](const InfObjCoatingParticle& p) {
    for (auto state : states) {
      if (p.state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<InfObjCoatingParticle>(prop, startLabel);
}

bool InfObjCoatingParticle::hasNbrInState(std::initializer_list<State> states)
    const {
  return labelOfFirstNbrInState(states) != -1;
}

int InfObjCoatingParticle::nextSurfaceDir() const {
  Q_ASSERT(state == State::Leader);

  int dir = labelOfFirstObjectNbr();
  while (hasObjectAtLabel(dir)) {
    dir = (dir + 5) % 6;
  }

  return dir;
}

bool InfObjCoatingParticle::hasFollowerChild() const {
  auto prop = [&](const InfObjCoatingParticle& p) {
    return p.state == State::Follower
        && (isContracted() ? pointsAtMyHead(p, p.dirToHeadLabel(p.moveDir))
                           : pointsAtMyTail(p, p.dirToHeadLabel(p.moveDir)));
  };

  return labelOfFirstNbrWithProperty<InfObjCoatingParticle>(prop) != -1;
}

InfObjCoatingSystem::InfObjCoatingSystem(uint numParticles, double holeProb) {
  Q_ASSERT(numParticles > 0);
  Q_ASSERT(0 <= holeProb && holeProb <= 1);

  std::set<Node> objNodes;  // Nodes occupied by object.
  std::set<Node> particleNodes;  // Nodes occupied by non-object particles.

  // Instantiate the object as a single line with turns. "Infinite" in this case
  // just means the object's surface is longer than the number of particles.
  Node objPos;
  while (objNodes.size() < numParticles * 2) {
    // Insert a new object particle at the given position.
    insert(new Object(objPos));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    int offset = (randInt(2, 5) + 3) % 6;
    objPos = objPos.nodeInDir(offset);
  }

  // Construct a forest structure of particles connected to the surface. Begin
  // with unoccupied positions above/adjacent to the surface as candidates.
  std::set<Node> candidates;
  for (auto objPos = objNodes.rbegin(); objPos != objNodes.rend(); ++objPos) {
    // Want some forest structure, so only include sqrt(n) positions on surface.
    if (candidates.size() > sqrt(numParticles)) {
      break;
    }

    for (int dir = 1; dir <= 2; ++dir) {
      const Node node = objPos->nodeInDir(dir);
      if (objNodes.find(node) == objNodes.end()
          && candidates.find(node) == candidates.end()) {
        candidates.insert(node);
      }
    }
  }

  while (particleNodes.size() < numParticles) {
    std::set<Node> lastAdded;
    for (auto candPos : candidates) {
      // Place a particle at the candidate position with probability 1 - hole.
      if (particleNodes.size() < numParticles && randBool(1 - holeProb)) {
        insert(new InfObjCoatingParticle(candPos, -1, randDir(), *this,
                                       InfObjCoatingParticle::State::Inactive));
        particleNodes.insert(candPos);
        lastAdded.insert(candPos);
      }
    }

    // Create new set of candidate nodes from the unoccupied positions adjacent
    // and above nodes occupied by newly added particles. Reuse last candidate
    // set if no particles were added, since it would be the same.
    if (lastAdded.size() > 0) {
      candidates.clear();
      for (auto prtPos : lastAdded) {
        for (int dir = 1; dir <= 2; ++dir) {
          const Node node = prtPos.nodeInDir(dir);
          if (objNodes.find(node) == objNodes.end()
              && particleNodes.find(node) == particleNodes.end()
              && candidates.find(node) == candidates.end()) {
            candidates.insert(node);
          }
        }
      }
    }
  }
}

bool InfObjCoatingSystem::hasTerminated() const {
  // Algorithm is terminated if all particles are on the surface (leaders) and
  // have contracted.
  for (auto p : particles) {
    auto iocp = dynamic_cast<InfObjCoatingParticle*>(p);
    if ((iocp->state != InfObjCoatingParticle::State::Leader) ||
        iocp->hasToken<InfObjCoatingParticle::ComplaintToken>()) {
      return false;
    }
  }

  return true;
}
