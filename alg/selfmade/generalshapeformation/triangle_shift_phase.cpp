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


void GSFParticle::triangle_shift_coordinatorActivate(){
    if (hasToken<triangle_shift_TriggerShiftToken>()){
        auto triggerToken = peekAtToken<triangle_shift_TriggerShiftToken>();

        if (triggerToken->_initiated){
            // confirmation token dat alles op z'n plek staat
            // aan het einde de originele coordinator als coordinator toewijzen
        }
        else {
            triggerToken->_initiated = true;
            int moveDir = triggerToken->_dir;

            std::cout << "h1" << std::endl;
            if (!(hasNbrAtLabel(moveDir)) && !(hasNbrAtLabel((moveDir + 3) % 6))) {
                auto shiftToken = std::make_shared<triangle_shift_ShiftToken>();

                std::cout << "h2" << std::endl;
                // do this only if triangle has to move left or right from perspective of coordinator
                shiftToken->_left = (triggerToken->_dir == 3) ? true : false;

                // token doorpasen langs de linkerzijde van de driehoek anders rechterzijde
                int passTo = (shiftToken->_left) ? (moveDir + 1) % 6: (moveDir + 5) % 6;
                shiftToken->_dirpassed = passTo;
                shiftToken->_level = 1;
                nbrAtLabel(passTo).putToken(shiftToken);


                // refactor deze zooi
                auto movementToken = std::make_shared<chain_MovementInitToken>();
                movementToken->_contract = true;
                movementToken->_lifetime = 0;
                movementToken->_dirpassed = shiftToken->_dirpassed;
                int dir = (shiftToken->_left) ? (shiftToken->_dirpassed + 5) % 6 : (shiftToken->_dirpassed + 1) % 6;
                movementToken->L.push(dir);
                putToken(movementToken);

            }
            else {
                // if not left or right, pass the coordinator token
                auto coordToken = std::make_shared<triangle_shift_CoordinatorToken>();
                int passCoordTo = -1;

                /*
                 * Determine the direction to which the coordinator should be passed
                */

                // move coordinator state to  left bottom vertex
                if (!hasNbrAtLabel(moveDir) && hasNbrAtLabel((moveDir + 3) % 6) && hasNbrAtLabel((moveDir + 2) % 6)) {
                // if pointing left upwards
                    passCoordTo = (moveDir + 2) % 6;
                    coordToken->_shiftdir = 4;
                }
                else if (hasNbrAtLabel(moveDir) && hasNbrAtLabel((moveDir + 5) % 6) && !hasNbrAtLabel((moveDir + 3) % 6)) {
                // if pointing right downwards
                    passCoordTo = (moveDir + 5) % 6;
                    coordToken->_shiftdir = 5;
                }

                // move coordinator state to right bottom vertex
                else if (!hasNbrAtLabel(moveDir) && hasNbrAtLabel((moveDir + 3) % 6) && hasNbrAtLabel((moveDir + 4) % 6)) {
                // if pointing right upwards
                    passCoordTo = (moveDir + 4) % 6;
                    coordToken->_shiftdir = 2;
                }
                else if (hasNbrAtLabel(moveDir) && hasNbrAtLabel((moveDir + 1) % 6) && !hasNbrAtLabel((moveDir + 3) % 6)) {
                // if pointing left downwards
                    passCoordTo = (moveDir + 1) % 6;
                    coordToken->_shiftdir = 5;
                }

                // Make sure coordinator has direction to pass coordinator token
                Q_ASSERT(passCoordTo >= 0);
                coordToken->_dirpassed = passCoordTo;
                nbrAtLabel(passCoordTo).putToken(coordToken);
                _state = State::CHAIN_FOLLOWER;
            }
        }
    }
}




void GSFParticle::triangle_shift_particleActivate(){
    if (hasToken<triangle_shift_ShiftToken>()) {
        auto shiftToken = takeToken<triangle_shift_ShiftToken>();
        int passTo = shiftToken->_dirpassed;
        if (hasNbrAtLabel(passTo)){
            nbrAtLabel(passTo).putToken(shiftToken);
        }
        _state = State::CHAIN_COORDINATOR;

        auto movementToken = std::make_shared<chain_MovementInitToken>();
        movementToken->_contract = true;
        movementToken->_lifetime = 0;
        movementToken->_dirpassed = shiftToken->_dirpassed;
        int dir = (shiftToken->_left) ? (shiftToken->_dirpassed + 5) % 6 : (shiftToken->_dirpassed + 1) % 6;
        movementToken->L.push(dir);
        putToken(movementToken);
    }

    if (hasToken<triangle_shift_CoordinatorToken>()) {
        auto coordToken = peekAtToken<triangle_shift_CoordinatorToken>();
        if (hasNbrAtLabel(coordToken->_dirpassed)){
            // remove token and pass it over to the next particle
            takeToken<triangle_shift_CoordinatorToken>();
            nbrAtLabel(coordToken->_dirpassed).putToken(coordToken);
        }
        else {
            auto triggerToken = std::make_shared<triangle_shift_TriggerShiftToken>();
            triggerToken->_dir = (coordToken->_dirpassed + coordToken->_shiftdir) % 6;
            triggerToken->_initiated = false;
            putToken(triggerToken);
            _state = State::COORDINATOR;
        }
    }
}
