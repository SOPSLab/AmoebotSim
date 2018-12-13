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
  // Finished particles do nothing.
  if (state == State::Finished) {
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
      if (hasObjectNbr() || hasNbrInState({State::Finished})) {
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
      if (hasObjectNbr() || hasNbrInState({State::Finished})) {
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
  if (state == State::Idle) {
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
  return hasObjectAtLabel(port) ||
      (hasNbrAtLabel(port) && nbrAtLabel(port).state == State::Finished);
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

  int dir = labelOfFirstNbrInState({State::Finished}, (moveDir + 1) % 6);
  if (dir == -1) {
    dir = labelOfFirstObjectNbr((moveDir + 1) % 6);
  }
  while (nbrIsObject(dir)) {
    dir = (dir + 5) % 6;
  }

  return dir;
}

// TODO: create a more dynamic surface generation.
FaultRepairSystem::FaultRepairSystem(uint numParticles, double holeProb,
                                     double branchFactor) {
  Q_ASSERT(numParticles > 0);
  Q_ASSERT(0 <= holeProb && holeProb <= 1);
  Q_ASSERT(0 <= branchFactor && branchFactor <= 1);

  std::set<Node> objNodes;  // Nodes occupied by object.
  std::set<Node> particleNodes;  // Nodes occupied by non-object particles.

  // Define the initial structural variables of the "rectangle" representing
  // the surface structure which will later contain faults.
  // surface represents how wide the structure is, depth represents how
  // "deep" the structure will go, and stretch represents the length of the
  // beginning and end sides of the structure.
  int surface = numParticles > 150 ?
        50 : numParticles > 50 ? numParticles / 3 : 20;
  int depth = -(surface / 3);
  int stretch = numParticles / 10;

  // Generate a 2x2 rectangle using the parameters defined previously
  for (int i = 0; i < surface + 2 * stretch; i++) {
    for (int j = 0; j > depth; j--) {
      Node* node = new Node(i, j);
      objNodes.insert(*node);
    }
  }

  // Define a set of variables that control the structure of the faults:
  // randHeight is a random integer representing how "tall" the fault will
  // initially be. prevHeight represents the height of the last Object of the
  // previous fault (from left-to-right order), which will be used to create
  // branches in the fault structures. Finally, currHeight is used to keep track
  // of the current height of the last inserted Object of the current fault, and
  // will be stored later in prevHeight at the end of the generation of the
  // current fault.
  int randHeight = randInt(0, -(depth + 2));
  int prevHeight = 0, currHeight = 0;
  for (auto pos = objNodes.begin(); pos != objNodes.end(); ) {
    // Check whether or not the current position node is a part of the beginning
    // stretch, ending stretch, left boundary, right boundary, or bottom
    // boundary of the initial rectangle structure. If so, insert an Object at
    // that position node.
    if ((pos->y == 0 &&
         (pos->x <= stretch - 1 || pos->x >= surface + stretch)) ||
        pos->x == stretch - 1 || pos->x == surface + stretch ||
        (pos->y == depth + 1 &&
         pos->x > stretch - 1 && pos->x < surface + stretch)) {
      insert(new Object(*pos));
      ++pos;
    // Check whether or not the current position node is located below the
    // beginning or ending stretches, and if so, delete them from the list of
    // potential locations for objects.
    } else if (pos->y != 0 && (pos->x <= stretch - 1 ||
                               pos->x >= surface + stretch)) {
      objNodes.erase(pos++);
    } else { // Below details how the faults are constructed
      // checkHeight is used to keep track of the current height within the
      // structure, and if it exceeds the maximum height, we move to the next
      // fault.
      int checkHeight = 0;
      // We iterate from the bottom boundary up to the maximum height allowed
      // for a single column in the structure, inserting Object entities for
      // the fault as we go.
      while (checkHeight < -(depth + 1)) {
        // Generate a boolean array which contains the status of the 3 neighbors
        // of the current position node in the directions {2, 3, 4}. If the
        // neighbors are in the system, their values are stored as true;
        // otherwise, if the neighboring position nodes have been removed, they
        // are stored as false
        bool nbrStatus[] = {false, false, false};
        for (int i = 2; i <= 4; i++) {
          nbrStatus[i - 2] =
              objNodes.find(pos->nodeInDir(i)) != objNodes.end();
        }
        // The two boolean variables below are used to determine whether or not
        // to insert an Object at the current position node. holeCheck is used
        // to avoid creating a "hole" in the resulting structure, i.e., an
        // isolated opening in the structure which cannot be reached by
        // particles because it is blocked from the particles by Object
        // entities. hasNbr is used to determine whether or not the current
        // position node has valid neighboring position nodes.
        bool holeCheck = nbrStatus[0] && !nbrStatus[1] && nbrStatus[2];
        bool hasNbr = nbrStatus[0] || nbrStatus[1] || nbrStatus[2];
        // Check whether or not the current position node is located directly
        // next to the right boundary. If so, we check whether or not its
        // neighbor at position 4 is valid, and if not, we delete the current
        // position node from objNodes. This check is done to avoid holes in the
        // structure.
        if (pos->x == surface + stretch - 1 && (!nbrStatus[2])) {
          objNodes.erase(pos++);
          checkHeight++;
        // The actions below attempt to create a fault of the specified
        // height from randHeight. This is done by inserting Object entities
        // into the position nodes (assuming the position nodes are valid, i.e.,
        // still contained in objNodes) until checkHeight equals randHeight,
        // if the current position node has valid neighbors, and inserting
        // an Object at the node will not create a hole in the structure.
        } else if (checkHeight < randHeight) {
          if (hasNbr && !holeCheck) {
            insert(new Object(*pos));
            ++pos;
            checkHeight++;
            currHeight = checkHeight;
          } else {
            objNodes.erase(pos++);
            checkHeight++;
          }
          if (prevHeight > 0) {
            prevHeight--;
          }
        // The actions below will insert Objects depending on the height of
        // the previous fault. The rationale here is that, if the previous fault
        // had inserted objects near our current height, then the current
        // position node is a possible position to insert an Object. This will
        // result in the branching behavior observed in fault creation.
        } else if (prevHeight > 0) {
          prevHeight--;
          // Here, we Generate a random integer that we will check against 10 *
          // the specified branching factor (from input). If the random integer
          // is less than the branching factor value * 10, then we will check
          // if there are neighboring particles to avoid having a floating
          // Object (an Object entity disconnected from the main structure).
          // If there are neighboring particles, we may insert; otherwise, we
          // delete the current position node from the list
          int rand = randInt(0, 10);
          if (rand > 10 * branchFactor) {
            if (hasNbr && !holeCheck) {
              insert(new Object(*pos));
              ++pos;
              currHeight = checkHeight;
            } else {
              objNodes.erase(pos++);
            }
          } else {
            objNodes.erase(pos++);
          }
          checkHeight++;
        } else {
          objNodes.erase(pos++);
          checkHeight++;
        }
      }
      randHeight = randInt(0, -(depth + 2));
      prevHeight = currHeight;
    }
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
    if (frp->state != FaultRepairParticle::State::Finished) {
      return false;
    }
  }

  return true;
}
