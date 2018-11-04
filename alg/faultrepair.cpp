#include <cmath>
#include <set>

#include "alg/faultrepair.h"

FaultRepairParticle::FaultRepairParticle(const Node head,
                                             const int globalTailDir,
                                             const int orientation,
                                             AmoebotSystem &system, State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    moveDir(-1),
    deltaOne(0),
    deltaTwo(0),
    surfaceVect(-1) {}

void FaultRepairParticle::activate() {
  if (isExpanded()) {
    if (state == State::Follower || state == State::Leader) {
      // If an expanded follower has no child or inactive neighbor, contract.
      if (!hasFollowerChild() && !hasNbrInState({State::Inactive})) {
        contractTail();
        return;
      } 
    }
  } else {  // Particle is contracted.
      if(true){
          //Determines adjacency configuration without needing to store a new integer
          //at most 6 lines will execute (states determined by binary tree)
          if(nbrIsObject(0)){ //2
              if(nbrIsObject(1)){ //14
                  if(nbrIsObject(2)){ //17
                      if(nbrIsObject(3)){ //22
                          if(nbrIsObject(4)){ //30
                              if(nbrIsObject(5)){
                                  //end
                              }
                              else{ //dead-end
                                  state = State::Object;
                              }
                          }
                          else{ //29
                              if(nbrIsObject(5)){ //dead-end
                                  state = State::Object;
                              }
                              //end
                          }
                      }
                      else{ //21
                          if(nbrIsObject(4)){ //28
                              if(nbrIsObject(5)){ //dead-end
                                  state = State::Object;
                              }
                              else{ //tunnel
                                  deltaOne = deltaTwo = 0;
                              }
                          }
                          //end
                      }
                  }
                  else{ //16
                      if(nbrIsObject(3)){ //20
                          if(nbrIsObject(4)){ //27
                              if(nbrIsObject(5)){ //dead-end
                                  state = State::Object;
                              }
                              else{ //tunnel
                                  deltaOne = deltaTwo = 0;
                              }
                          }
                          else{ //26
                              if(nbrIsObject(5)){ //tunnel
                                  deltaOne = deltaTwo = 0;
                              }
                              //end
                          }
                      }
                      //end
                  }
              }
              else{ //13
                  if(nbrIsObject(2)){ //15
                      if(nbrIsObject(3)){ //19
                          if(nbrIsObject(4)){ //25
                              if(nbrIsObject(5)){ //dead-end
                                  state = State::Object;
                              }
                              else{ //tunnel
                                  deltaOne = deltaTwo = 0;
                              }
                          }
                          else{ //24
                              if(nbrIsObject(5)){ //tunnel
                                  deltaOne = deltaTwo = 0;
                              }
                              //end
                          }
                      }
                      else{ //18
                          if(nbrIsObject(4)){ //23
                              if(nbrIsObject(5)){ //tunnel
                                  deltaOne = deltaTwo = 0;
                              }
                              //end
                          }
                          //end
                      }
                  }
                  //end
              }
          }
          else{ //1
              if(nbrIsObject(1)){ //3
                  if(nbrIsObject(2)){ //5
                      if(nbrIsObject(3)){ //8
                          if(nbrIsObject(4)){ //12
                              if(nbrIsObject(5)){ //dead-end
                                  state = State::Object;
                              }
                              //end
                          }
                          else{ //11
                              if(nbrIsObject(5)){ //tunnel
                                  deltaOne = deltaTwo = 0;
                              }
                              //end
                          }
                      }
                      else{ //7
                          if(nbrIsObject(4)){ //10
                              if(nbrIsObject(5)){ //tunnel
                                  deltaOne = deltaTwo = 0;
                              }
                              //end
                          }
                          //end
                      }
                  }
                  else{ //4
                      if(nbrIsObject(3)){ //6
                          if(nbrIsObject(4)){ //9
                              if(nbrIsObject(5)){ //tunnel
                                  deltaOne = deltaTwo = 0;
                              }
                              //end
                          }
                          //end
                      }
                      //end
                  }
              }
              //end
          }
       }
    if (state == State::Inactive) {
      // Inactive particles need to first join the spanning tree.
      if (hasNbrInState({State::Object})) {
        state = State::Leader;
        moveDir = nextSurfaceDir();
        surfaceVect = moveDir;
        return;
      } else if (state != State::Leader && hasNbrInState({State::Leader, State::Follower})) {
        state = State::Follower;
        moveDir = labelOfFirstNbrInState({State::Leader, State::Follower});
        return;
      }
    } else if (state == State::Follower) {
      if (hasNbrInState({State::Object})) {
        // If a follower has followed its spanning tree to the surface, become a
        // leader, removing follow direction and calculating move direction.
        state = State::Leader;
        moveDir = nextSurfaceDir();
        surfaceVect = moveDir;
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
      moveDir = nextSurfaceDir();

      if(surfaceVect != moveDir) {
        deltaTwo = deltaOne;
        deltaOne = surfaceVect - moveDir;
        if(deltaOne < -3) {
          deltaOne += 6;
        } else if (deltaOne > 3) {
          deltaOne -= 6;
        }
        surfaceVect = moveDir;
      }
      if ((deltaTwo == -1 && deltaOne == 2) ||
          (deltaTwo == -2 && deltaOne == 2) ||
          (deltaTwo == 1 && (deltaOne == 1 || deltaOne == 2)) ||
          (deltaTwo == 2 && (deltaOne == 1 || deltaOne == 2))) {
        state = State::Object;
        return;
      }
      if (!hasNbrAtLabel(moveDir)) {
        expand(moveDir);
        if(deltaOne == 2 && !hasNbrAtLabel((moveDir+1)%6)){
           contractHead();
           state = State::Object;
        }
        return;
      } else if (hasTailAtLabel(moveDir)) {
        // If there is an expanded particle ahead, handover expand
        push(moveDir);
        return;
      } else
        return;
    }
  }
}

int FaultRepairParticle::headMarkColor() const {
  switch(state) {
  case State::Object:   return 0x000000;
  case State::Inactive: return -1;
  case State::Follower: return 0x0000ff;
  case State::Leader:   return 0xff0000;
  }

  return -1;
}

int FaultRepairParticle::headMarkDir() const {
  return (state == State::Leader || state == State::Follower) ? moveDir : -1;
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
  text += [this](){
    switch(state) {
    case State::Object:   return "object\n";
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

/*int FaultRepairParticle::nextSurfaceDir() const {
  Q_ASSERT(state == State::Leader);

  int dir = labelOfFirstNbrInState({State::Object},(moveDir+4)%6); //<---initial port?
  while (hasNbrAtLabel(dir) && nbrAtLabel(dir).state == State::Object) {
    dir = (dir +5) % 6;
  }

  return dir;
}*/

int FaultRepairParticle::nextSurfaceDir() const {
  Q_ASSERT(state == State::Leader);

  int dir = labelOfFirstNbrInState({State::Object}, (moveDir+4)%6);
  while (hasNbrAtLabel(dir) && nbrAtLabel(dir).state == State::Object) {
    dir = (dir + 5) % 6;
  }

  return dir;
}

bool FaultRepairParticle::nbrIsObject(int port) const {
  if (hasNbrAtLabel(port) && nbrAtLabel(port).state == State::Object) {
    return true;
  }

  return false;
}

bool FaultRepairParticle::hasFollowerChild() const {
  auto prop = [&](const FaultRepairParticle& p) {
    return p.state == State::Follower
        && (isContracted() ? pointsAtMyHead(p, p.dirToHeadLabel(p.moveDir))
                           : pointsAtMyTail(p, p.dirToHeadLabel(p.moveDir)));
  };

  return labelOfFirstNbrWithProperty<FaultRepairParticle>(prop) != -1;
}

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
                                       FaultRepairParticle::State::Inactive));
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
  // Algorithm is terminated if all particles are on the surface (object) and
  // have contracted.
  for (auto p : particles) {
    auto iocp = dynamic_cast<FaultRepairParticle*>(p);
    if ((iocp->state != FaultRepairParticle::State::Object)) {
      return false;
    }
  }

  return true;
}
