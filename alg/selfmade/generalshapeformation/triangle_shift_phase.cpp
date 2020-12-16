#include "../generalshapeformation.h"
#include <iostream>

using namespace std;

void GSFParticle::triangle_shift_activate(){
    if(_state == State::COORDINATOR){
        triangle_shift_coordinatorActivate();

    }
    else {
        triangle_shift_particleActivate();
    }
}


//  struct triangle_shift_TriggerShiftToken: public Token{int _dir; bool _initiated;};
//  struct triangle_shift_ShiftToken: public Token{int _level; int _left; int _dirpassed;};



void GSFParticle::triangle_shift_coordinatorActivate(){
    if (hasToken<triangle_shift_TriggerShiftToken>()){
        auto triggerToken = peekAtToken<triangle_shift_TriggerShiftToken>();

        if (triggerToken->_initiated){
            // confirmation token dat alles op z'n plek staat
            // aan het einde de originele coordinator als coordinator toewijzen
        }
        else {
            triggerToken->_initiated = true;


            auto shiftToken = std::make_shared<triangle_shift_ShiftToken>();
            int moveDir = triggerToken->_dir;


            if (!(hasNbrAtLabel(moveDir)) && !(hasNbrAtLabel((moveDir + 3) % 6))) {
                // do this only if triangle has to move left or right from perspective of coordinator
                shiftToken->_left = (triggerToken->_dir == 3) ? true : false;
                putToken(shiftToken);
                // token doorpasen langs de linkerzijde van de driehoek anders rechterzijde
                int passTo = (shiftToken->_left) ? (moveDir + 1) % 6: (moveDir + 5) % 6;
                shiftToken->_dirpassed = passTo ;
                nbrAtLabel(passTo).putToken(shiftToken);
            }
            else {



                // cannot move
                //pass down coordinator-token to the corners

               // doorpasen naar rechterzijde
//               int passTo = (movementDir + 5) % 6;
//               nbrAtLabel(passTo).putToken(shiftToken);


            }
            // check if direction is left or right




            //
        }


    }
}

void GSFParticle::triangle_shift_particleActivate(){
    if (hasToken<triangle_shift_ShiftToken>()) {
        auto shiftToken = peekAtToken<triangle_shift_ShiftToken>();
        int passTo = shiftToken->_dirpassed;
        if (hasNbrAtLabel(passTo)){
            nbrAtLabel(passTo).putToken(shiftToken);
        }
        _state = State::CHAIN_COORDINATOR;
        auto chainCoorToken = std::make_shared<chain_ChainToken>();
        putToken(chainCoorToken);
    }
}
