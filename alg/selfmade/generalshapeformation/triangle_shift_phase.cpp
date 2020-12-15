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
            // if direction is to the left or right (3 = left, 6 = right)

            if (!(hasNbrAtLabel(moveDir)) && !(hasNbrAtLabel(moveDir + 3) % 6)) {
                shiftToken->_left = (triggerToken->_dir == 3) ? true : false;
                putToken(shiftToken);
                int passTo;
                // token doorpasen langs de linkerzijde van de driehoek
                if (shiftToken->_left) {
                    passTo = (moveDir + 1) % 6;
                    nbrAtLabel(passTo).putToken(shiftToken);
                }
                // token langs de rechterzijde van de driehoek
                else {
                    passTo = (moveDir + 5) % 6;
                    nbrAtLabel(passTo).putToken(shiftToken);
                }
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
        auto chainCoorToken = std::make_shared<chain_ChainToken>();
//        putToken()
    }
}
