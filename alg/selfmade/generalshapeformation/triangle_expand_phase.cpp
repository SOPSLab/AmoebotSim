#include "../generalshapeformation.h"

void GSFParticle::triangle_expand_activate(){
    if(_state == State::COORDINATOR){
        triangle_expand_coordinatorActivate();
    }else{
        triangle_expand_particleActivate();
    }
    return;
}

void GSFParticle::triangle_expand_coordinatorActivate(){
    if(hasToken<triangle_expand_TriggerExpandToken>()){
        auto triggerToken = peekAtToken<triangle_expand_TriggerExpandToken>();
        if(triggerToken->_initated){
            if(hasToken<triangle_expand_ConfirmExpandToken>()){
                // Coordinator particle was waiting for confirmation, and received at now. So terminate expand operation
                //  and update triangleDirection
                takeToken<triangle_expand_ConfirmExpandToken>();
                auto expandToken = takeToken<triangle_expand_TriggerExpandToken>();
                if(expandToken->_left){
                    _triangleDirection = (_triangleDirection + 5) % 6;
                }else{
                    _triangleDirection = (_triangleDirection + 1) % 6;
                }
            }

        }else{
            triggerToken->_initated = true;

            // Triangle has not yet been initiated, so initiate it.
            int movementDir, dirpassed, confirmationDir;

            // Set token attributes based on whether the triangle has to expand left or right.
            if(triggerToken->_left){
                dirpassed = _triangleDirection;
                movementDir = (_triangleDirection + 4 )% 6;
                confirmationDir = (dirpassed + 2) % 6;
            }else{
                dirpassed = (_triangleDirection + 1) % 6;
                movementDir = (_triangleDirection + 3 )% 6;
                confirmationDir = (movementDir + 2) % 6;
            }

            auto expandToken = std::make_shared<triangle_expand_ExpandToken>();
            expandToken->_level = 1;
            expandToken->_dirpassed = dirpassed;
            expandToken->_movementdir = movementDir;
            expandToken->_confirmationdir = confirmationDir;

            // Pass token to first particle in the chain
            if(hasNbrAtLabel(dirpassed)){
                nbrAtLabel(dirpassed).putToken(expandToken);
            }
        }
    }
}

/**
 * Handle triangle_expand for a non-coordinator
 * */
void GSFParticle::triangle_expand_particleActivate(){

    if(hasToken<triangle_expand_ExpandToken>()){
        std::shared_ptr<triangle_expand_ExpandToken> expandToken = takeToken<triangle_expand_ExpandToken>();
        triangle_expand_createMovementInitToken(expandToken);

        triangle_expand_forwardExpandToken(expandToken);
    }

    if(hasToken<triangle_expand_ConfirmExpandToken>()){
        triangle_expand_handleConfirmExpandToken();
    }
}

/**
 * Forward the confirmation token towards the coordinator, iff you are no longer a chain_coordinator, meaning your chain has finished moving.
 * @brief GSFParticle::triangle_expand_handleConfirmExpandToken
 * @param confirmToken
 */
void GSFParticle::triangle_expand_handleConfirmExpandToken(){
    if(_state != State::CHAIN_COORDINATOR && !hasToken<chain_MovementInitToken>()){
        auto confirmToken = peekAtToken<triangle_expand_ConfirmExpandToken>();
        //  If you are not a chain coordinator anymore, but have the confirm expand token, you are done and can confirm (or pass confirmation).
        int dirpassed = confirmToken->_dirpassed;
        if(hasNbrAtLabel(dirpassed)){
            confirmToken = takeToken<triangle_expand_ConfirmExpandToken>();
            nbrAtLabel(dirpassed).putToken(confirmToken);
        }
    }
}

/**
 * Create the MovementInitToken to start the movement along the path required for triangle expansion
 * */
void GSFParticle::triangle_expand_createMovementInitToken(std::shared_ptr<triangle_expand_ExpandToken> expandToken){
    auto movementToken = std::make_shared<chain_MovementInitToken>();
    movementToken->_contract = true;
    movementToken->_lifetime = 0;
    movementToken->_dirpassed = expandToken->_dirpassed;
    for(int i = 0; i < expandToken->_level; i++){
        movementToken->L.push(expandToken->_movementdir);
    }

    putToken(movementToken);
}


void GSFParticle::triangle_expand_forwardExpandToken(std::shared_ptr<triangle_expand_ExpandToken> expandToken){
    if(hasNbrAtLabel(expandToken->_dirpassed)){
        expandToken->_level++;
        nbrAtLabel(expandToken->_dirpassed).putToken(expandToken);
    }else{
        // This particle is the last in the triangle edge
        auto confirmToken = std::make_shared<triangle_expand_ConfirmExpandToken>();
        confirmToken->_dirpassed = expandToken->_confirmationdir;
        putToken(confirmToken);

    }
}
