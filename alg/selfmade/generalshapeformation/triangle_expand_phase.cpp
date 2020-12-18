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
            // TODO: Wait until confirmation is received of expansion completion
        }else{
            triggerToken->_initated = true;

            // Triangle has not yet been initiated, so initiate it.
            int movementDir, dirpassed;

            // Set token attributes based on whether the triangle has to expand left or right.
            if(triggerToken->_left){
                dirpassed = _triangleDirection;
                movementDir = (_triangleDirection + 4 )% 6;
            }else{
                dirpassed = (_triangleDirection + 1) % 6;
                movementDir = (_triangleDirection + 3 )% 6;
            }

            auto expandToken = std::make_shared<triangle_expand_ExpandToken>();
            expandToken->_level = 1;
            expandToken->_dirpassed = dirpassed;
            expandToken->_movementdir = movementDir;

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
    }
}
