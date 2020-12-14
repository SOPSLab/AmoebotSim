/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/demo/pa.h"

PAParticle::PAParticle(const Node& head, const int globalTailDir,
                                     const int orientation,
                                     AmoebotSystem& system,
                                     State state)
    : AmoebotParticle(head, globalTailDir, orientation, system),
    _state(state){
    _movingDir = 1;
}

void PAParticle::activate() {
    if(_state == State::Leader){
      if(isContracted()){
          if(canExpand(_movingDir)){
//              expand(_movingDir);
          }

          if(randBool(0.2)){
            _movingDir++;
          }
      }else{
          contractTail();
      }
    }else if (_state == State::Follower) {

    }else{ // _state == Silly
      if(!tryToConnectNeighbor()){
          int expandDir = randDir();
          expandDir = 1;
          if (canExpand(expandDir)) {
            expand(expandDir);
          }
      }
    }
}



int PAParticle::headMarkColor() const {
    if(_state == State::Leader) {
        return 0x00FF00;
    }else if (_state == State::Follower){
        return 0xfa2222;
    }
    return 0x000000;
}

int PAParticle::tailMarkColor() const {
  return headMarkColor();
}

QString PAParticle::inspectionText() const {
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
          case State::Silly: return "silly\n";
        }
  }();
  text += " nbr: " + QString::number(labelOfFirstObjectNbr());
  text += "  counter: Ook al niet";

  return text;
}

PAParticle& PAParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<PAParticle>(label);
}


PASystem::PASystem(unsigned int numParticles) {
  int sideLength = std::sqrt(numParticles) + 1;
  std::set<Node> occupied;
  while (occupied.size() < numParticles) {
    // First, choose an x and y position at random from the (i) and (ii) bounds.
    int x = randInt(0, sideLength);
    int y = randInt(0, sideLength);
    Node node(x, y);

    // If the node satisfies (iii) and is unoccupied, place a particle there.
    if (occupied.find(node) == occupied.end()) {
      insert(new PAParticle(node, -1, randDir(), *this, PAParticle::State::Silly));
      occupied.insert(node);
    }
  }
}
