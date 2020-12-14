#include "alg/demo/pa.h"

bool PAParticle::tryToConnectNeighbor() {
    const int labelLimit = isContracted() ? 6 : 10;
    bool hasNbr = false;
    bool hasNonSillyNbr = false;

    for (int label = 0; label < labelLimit; label++) {
      if (hasNbrAtLabel(label)) {
          hasNbr = true;
          State nbrState = nbrAtLabel(label)._state;
          if(nbrState == State::Leader || nbrState == State::Follower){
              hasNonSillyNbr |= true;
          }
      }
    }

    if(hasNbr){
        if(hasNonSillyNbr){
            _state = State::Follower;
        }else{
            _state = State::Leader;
        }
        return true;
    }

    return false;
}
