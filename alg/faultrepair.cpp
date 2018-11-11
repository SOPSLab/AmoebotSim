#include <cmath>
#include <set>

#include "alg/faultrepair.h"

FaultRepairParticle::FaultRepairParticle(const Node head,
                                         const int globalTailDir,
                                         const int orientation,
                                         AmoebotSystem &system, State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    delta1(0),
    delta2(0),
    surfaceVec(-1),
    moveDir(-1) {}

void FaultRepairParticle::activate() {
  // Objects and finished particles do nothing.
  if (state == State::Object || state == State::Finished) {
    return;
  }

  if (isExpanded()) {
    // First, handle the override case where the particle contracts back.
    // Otherwise, contract if safe to do so.
    // TODO: figure out if this first expanded case is necessary.
    /*if (state == State::Root && delta1 == 2
        && !nbrIsObject((moveDir + 1) % 6)) {
      contractHead();
      state = State::Finished;
      return;
    } else*/ if (state == State::Follower || state == State::Root) {
      if (!hasFollowerChild() && !hasNbrInState({State::Idle})) {
        contractTail();
        return;
      }
    }
  } else {  // Particle is contracted.
    // Handle tunnel cases.
    if (isAtDeadEnd()) {
      state = State::Finished;
    } else if (isInTunnel()) {
      delta1 = 0;
      delta2 = 0;
    }

    if (state == State::Idle) {
      // Idle particles need to first join the spanning tree.
      if (hasNbrInState({State::Object, State::Finished})) {
        state = State::Root;
        moveDir = nextSurfaceDir();
        surfaceVec = moveDir;
        return;
      } else if (hasNbrInState({State::Root, State::Follower})) {
        state = State::Follower;
        moveDir = labelOfFirstNbrInState({State::Root, State::Follower});
        return;
      }
    } else if (state == State::Follower) {
      if (hasNbrInState({State::Object, State::Finished})) {
        // If a follower has followed its spanning tree to the surface, become a
        // root, removing follow direction and calculating move direction.
        state = State::Root;
        moveDir = nextSurfaceDir();
        surfaceVec = moveDir;
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
    } else if (state == State::Root) {
      // Calculate next surface direction and resulting deltas.
      moveDir = nextSurfaceDir();
      if (surfaceVec != moveDir) {
        delta2 = delta1;
        delta1 = surfaceVec - moveDir;
        if (delta1 < -3) {
          delta1 += 6;
        } else if (delta1 > 3) {
          delta1 -= 6;
        }
        surfaceVec = moveDir;
      }

      // Check the delta finishing conditions. Otherwise, move if possible.
      if ((delta2 == -1 && delta1 == 2) || (delta2 == -2 && delta1 == 2)
          || (delta2 == 1 && (delta1 == 1 || delta1 == 2))
          || (delta2 == 2 && (delta1 == 1 || delta1 == 2))) {
        state = State::Finished;
        return;
      } else if (canExpand(moveDir)) {
        expand(moveDir);
        return;
      } else if (hasTailAtLabel(moveDir)) {
        push(moveDir);
        return;
      }
    }
  }
}

int FaultRepairParticle::headMarkColor() const {
  switch(state) {
    case State::Object:   return 0x000000;
    case State::Idle:     return -1;
    case State::Follower: return 0x0000ff;
    case State::Root:     return 0xff0000;
    case State::Finished: return 0x00ff00;
  }

  return -1;
}

int FaultRepairParticle::headMarkDir() const {
  return (state == State::Root || state == State::Follower) ? moveDir : -1;
}

int FaultRepairParticle::tailMarkColor() const {
  return headMarkColor();
}

QString FaultRepairParticle::inspectionText() const {
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
                      + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Local Info:\n";
  text += "  state: ";
  if (state == State::Object) {
    text += "object\n";
  } else if (state == State::Idle) {
    text += "idle\n";
  } else if (state == State::Follower) {
    text += "follower\n";
  } else if (state == State::Root) {
    text += "root\n";
  } else if (state == State::Finished) {
    text += "finished\n";
  }
  text += "  delta1: " + QString::number(delta1) + "\n";
  text += "  delta2: " + QString::number(delta2) + "\n";
  text += "  surfaceVec: " + QString::number(surfaceVec) + "\n";
  text += "  moveDir: " + QString::number(moveDir) + "\n";

  return text;
}

FaultRepairParticle& FaultRepairParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<FaultRepairParticle>(label);
}

int FaultRepairParticle::labelOfFirstNbrInState(
    std::initializer_list<State> states, int startLabel) const {
  auto prop = [&](const FaultRepairParticle& p) {
    for (auto state : states) {
      if (p.state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<FaultRepairParticle>(prop, startLabel);
}

bool FaultRepairParticle::hasNbrInState(std::initializer_list<State> states)
    const {
  return labelOfFirstNbrInState(states) != -1;
}

bool FaultRepairParticle::hasFollowerChild() const {
  auto prop = [&](const FaultRepairParticle& p) {
    return p.state == State::Follower
        && (isContracted() ? pointsAtMyHead(p, p.dirToHeadLabel(p.moveDir))
                           : pointsAtMyTail(p, p.dirToHeadLabel(p.moveDir)));
  };

  return labelOfFirstNbrWithProperty<FaultRepairParticle>(prop) != -1;
}

bool FaultRepairParticle::nbrIsObject(int port) const {
  return hasNbrAtLabel(port) && (nbrAtLabel(port).state == State::Object ||
                                 nbrAtLabel(port).state == State::Finished);
}

bool FaultRepairParticle::isInTunnel() const {
  Q_ASSERT(isContracted());

  // Find a port incident to an object/finished particle that has an unoccupied
  // node immediately clockwise. Note that this must exist, since a particle is
  // never surrounded by object/finished particles.
  int dir = -1;
  for (int i : uniqueLabels()) {
    if (nbrIsObject(i) && !nbrIsObject((i + 5) % 6)) {
      dir = i;
    }
  }

  bool seenNonObj = false;
  for (int offset = 1; offset < 6; ++offset) {
    if (nbrIsObject((dir + offset) % 6)) {
      if (seenNonObj) {
        return true;
      }
    } else {
      seenNonObj = true;
    }
  }

  return false;
}

bool FaultRepairParticle::isAtDeadEnd() const {
  Q_ASSERT(isContracted());

  // Count number of object neighbors.
  uint numObjNbrs = 0;
  for (int label : uniqueLabels()) {
    if (nbrIsObject(label)) {
      numObjNbrs++;
    }
  }

  return numObjNbrs == 5;
}

int FaultRepairParticle::nextSurfaceDir() const {
  Q_ASSERT(state == State::Root);

  int dir = labelOfFirstNbrInState({State::Object, State::Finished},
                                   (moveDir + 1) % 6);
  while (nbrIsObject(dir)) {
    dir = (dir + 5) % 6;
  }

  return dir;
}

// TODO: create a more dynamic surface generation.
FaultRepairSystem::FaultRepairSystem(uint numParticles, double holeProb) {
  Q_ASSERT(numParticles > 0);
  Q_ASSERT(0 <= holeProb && holeProb <= 1);

  std::set<Node> objNodes;  // Nodes occupied by object.
  std::set<Node> particleNodes;  // Nodes occupied by non-object particles.

  // Instantiate the object as a single line with turns. "Infinite" in this case
  // just means the object's surface is longer than the number of particles.
  Node objPos;
  for(int i = 0; i < 20; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, randDir(), *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    int offset = (randInt(2, 5) + 3) % 6;
    objPos = objPos.nodeInDir(offset);
  }

  for(int i = 0; i < 10; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 0, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(0);
  }

  for(int i = 0; i < 6; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 4, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(4);
  }

  for(int i = 0; i < 2; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 3, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(3);
  }

  for(int i = 0; i < 4; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 1, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(1);
  }

  for(int i = 0; i < 6; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 3, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(3);
  }

  for(int i = 0; i < 10; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 4, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(4);
  }

  for(int i = 0; i < 10; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 0, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(0);
  }

  for(int i = 0; i < 2; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 5, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(5);
  }

  for(int i = 0; i < 2; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 3, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(3);
  }

  for(int i = 0; i < 6; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 5, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(5);
  }

  for(int i = 0; i < 12; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 1, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(1);
  }

  for(int i = 0; i < 6; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 5, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(5);
  }

  for(int i = 0; i < 6; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 1, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(1);
  }

  for(int i = 0; i <2; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 0, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(0);
  }

  for(int i = 0; i < 6; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 4, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(4);
  }

  for(int i = 0; i < 8; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 0, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(0);
  }

  for(int i = 0; i < 4; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 2, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(2);
  }

  for(int i = 0; i < 8; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 0, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(0);
  }

  for(int i = 0; i < 10; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 2, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(2);
  }

  for(int i = 0; i < 10; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, 0, *this,
                                     FaultRepairParticle::State::Object));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    objPos = objPos.nodeInDir(0);
  }

  for(int i = 0; i < 20; i++) {
    // Insert a new object particle at the given position.
    insert(new FaultRepairParticle(objPos, -1, randDir(), *this,
                                     FaultRepairParticle::State::Object));
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
        insert(new FaultRepairParticle(candPos, -1, randDir(), *this,
                                       FaultRepairParticle::State::Idle));
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

bool FaultRepairSystem::hasTerminated() const {
  // Algorithm is terminated if all particles are finished.
  for (auto p : particles) {
    auto frp = dynamic_cast<FaultRepairParticle*>(p);
    if (frp->state != FaultRepairParticle::State::Object
        && frp->state != FaultRepairParticle::State::Finished) {
      return false;
    }
  }

  return true;
}
