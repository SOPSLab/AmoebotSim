#include "../generalshapeformation.h"

// All methods required to perform a Chain movement

void GSFParticle::chain_activate(){
    if(hasToken<ChainToken>()){
        Q_ASSERT(_state == State::CHAIN_COORDINATOR || _state == State::COORDINATOR);
        chain();
    }
    //Token to initialize a certian movement pattern. This is just used for debugging.
    //Should be replaced by a token of a specific primitive for a certian result.
    //Can give an idea of how to setup a movement primitive however.
    if(hasToken<MovementInitToken>()){
        auto token = takeToken<MovementInitToken>();
        //pass token to next particle allong the side of the triangle if necissary
        //and put chaintoken on this particle
        if(hasNbrAtLabel(token->_dirpassed) && token->_lifetime>0){
            token->_lifetime--;
            int dir = token->_dirpassed;
            nbrAtLabel(dir).putToken(token);
        }
        auto t = std::make_shared<ChainToken>();
        t->_contract = token->_contract;
        t->L = token->L;
        putToken(t);
        _ldrlabel = -1;

        //send a token to followers with their respective depth value
        if(_state != State::COORDINATOR){
            _state = State::CHAIN_COORDINATOR;
            if(hasNbrAtLabel((token->_dirpassed + 2) % 6)){
                auto t = std::make_shared<DepthToken>();
                t->_passeddir = (token->_dirpassed + 2) % 6;
                t->_depth = 1;
                nbrAtLabel((token->_dirpassed + 2) % 6).putToken(t);
            } else {
                auto t = std::make_shared<DepthToken>();
                t->_passeddir = (token->_dirpassed - 2) % 6;
                t->_depth = 1;
                nbrAtLabel((token->_dirpassed - 2) % 6).putToken(t);
            }
        }
    }

    // used for ensure that a chain has descending depth
    // depth i means that some particle is the ith particle
    // in a chain
    if(hasToken<DepthToken>()){
        Q_ASSERT(_state == State::FOLLOWER);
        auto token = takeToken<DepthToken>();
        _ldrlabel = (token->_passeddir+3)%6;
        _depth = token->_depth;
        token->_depth++;
        if(hasNbrAtLabel(token->_passeddir)){
            nbrAtLabel(token->_passeddir).putToken(token);
        }
    }

    //if follower is contracted, send confirmation token allong the chain
    if(hasToken<ConfirmContractToken>()){
        auto token = takeToken<ConfirmContractToken>();
        if(_state == State::FOLLOWER){
            if(isContracted()){
                if(_ldrlabel>-1){
                    Q_ASSERT(hasNbrAtLabel(_ldrlabel));
                    Q_ASSERT(nbrAtLabel(_ldrlabel)._depth == _depth-1);
                    Q_ASSERT(nbrAtLabel(_ldrlabel)._level == _level);
                    nbrAtLabel(_ldrlabel).putToken(token);
                }
            }
        }
    }

    //If a contract token is receive a particle in the chain wil try to contract
    if(hasToken<ContractToken>()){
        Q_ASSERT(_state == State::FOLLOWER);
        auto token = peekAtToken<ContractToken>();
        if(isExpanded()){
            if(!(_level == _depth)){
                for(int label : tailLabels()){
                    if(hasNbrAtLabel(label) && nbrAtLabel(label)._level == _level &&
                            nbrAtLabel(label)._depth == _depth+1){
                        if(canPull(label)){
                            nbrAtLabel(label)._ldrlabel =
                                    dirToNbrDir(nbrAtLabel(label), (tailDir() + 3) % 6);
                            pull(label);
                            token = takeToken<ContractToken>();
                        } else {
                            nbrAtLabel(label).putToken(token);
                        }
                    }
                }
            } else {
                takeToken<ContractToken>();
                contractTail();
                if(token->_final){
                    if(isContracted()){
                        Q_ASSERT(hasNbrAtLabel(_ldrlabel));
                        auto t = std::make_shared<ConfirmContractToken>();
                        nbrAtLabel(_ldrlabel).putToken(t);
                    }
                }
            }
        } else {
            for(int label : headLabels()){
                if(hasNbrAtLabel(label) && nbrAtLabel(label)._level == _level &&
                        nbrAtLabel(label)._depth == _depth+1){
                    token = takeToken<ContractToken>();
                    nbrAtLabel(label).putToken(token);
                }
            }
        }
    }
}

void GSFParticle::chain(){
    auto token = peekAtToken<ChainToken>();
    //expend to space if the chain coordinator is contracted
    if(isContracted()){
        //take top direction of token
        if(!token->L.empty()){
            int dir = token->L.top();
            token->L.pop();
            if(canExpand(dir)){
                expand(dir);
            }
        } else {
            if(token->_contract){
                if(_state == State::COORDINATOR){
                    auto token = takeToken<ChainToken>();
                } else {
                    for(int label : headLabels()){
                        if(hasNbrAtLabel(label) && nbrAtLabel(label)._level == _level&&
                                nbrAtLabel(label)._depth == _depth+1){
                            auto t = std::make_shared<ContractToken>();
                            t->_final = true;
                            nbrAtLabel(label).putToken(t);
                        }
                    }
                }
            }
        }
    //otherwise pull next node in the chain
    } else {
        if(!token->L.empty()){
            for(int label : tailLabels()){
                if(hasNbrAtLabel(label) && nbrAtLabel(label)._level == _level &&
                        nbrAtLabel(label)._depth == _depth+1){
                    if(canPull(label)){
                        auto part = nbrAtLabel(label);
                        nbrAtLabel(label)._ldrlabel =
                                dirToNbrDir(nbrAtLabel(label), (tailDir() + 3) % 6);
                        pull(label);
                        _sent_pull = false;
                    } else {
                        if(!_sent_pull){
                            _sent_pull = true;
                            auto t = std::make_shared<ContractToken>();
                            t->_final = false;
                            nbrAtLabel(label).putToken(t);
                        }
                    }
                    break;
                }
            }
        } else {
            //Contract the entire chain if so desired
            if (token->_contract){
                if(_state == State::COORDINATOR){
                    contractTail();
                    auto token = takeToken<ChainToken>();
                } else {
                    for(int label : tailLabels()){
                        if(hasNbrAtLabel(label) && nbrAtLabel(label)._level == _level&&
                                nbrAtLabel(label)._depth == _depth+1){
                            if(canPull(label)){
                                auto t = std::make_shared<ContractToken>();
                                t->_final = true;
                                nbrAtLabel(label).putToken(t);

                                nbrAtLabel(label)._ldrlabel =
                                            dirToNbrDir(nbrAtLabel(label), (tailDir() + 3) % 6);
                                pull(label);
                            } else {
                                auto t = std::make_shared<ContractToken>();
                                t->_final = true;
                                nbrAtLabel(label).putToken(t);
                            }
                        }
                    }
                }
            //otherwise stop chain execution
            } else {
                if(_state != State::COORDINATOR){
                    auto token = takeToken<ChainToken>();
                    _state = State::FOLLOWER;
                    _ldrlabel = -1;
                } else {
                    auto token = takeToken<ChainToken>();
                    _ldrlabel = -1;
                }
            }
        }
    }
    if(hasToken<ConfirmContractToken>()){
        auto token = takeToken<ChainToken>();
        auto token2 = takeToken<ConfirmContractToken>();
        _state = State::FOLLOWER;
        _ldrlabel = -1;
    }
}
