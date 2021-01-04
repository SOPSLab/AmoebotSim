#include "../generalshapeformation.h"
#include <QDebug>

// All methods required to perform a Chain movement
void GSFParticle::chain_activate(){
    if(hasToken<chain_ChainToken>()){
        chain_handleChainToken();
    }

    //Token to initialize a certian movement pattern. This is just used for debugging.
    //Should be replaced by a token of a specific primitive for a certian result.
    //Can give an idea of how to setup a movement primitive however.
    if(hasToken<chain_MovementInitToken>()){
        chain_handleMovementInitToken();
    }

    // used for ensure that a chain has descending depth
    // depth i means that some particle is the ith particle
    // in a chain
    if(hasToken<chain_DepthToken>()){
        chain_handleDepthToken();
    }

    //if follower is contracted, send confirmation token allong the chain
    if(hasToken<chain_ConfirmContractToken>()){
        chain_handleConfirmContractToken();
    }

    //If a contract token is receive a particle in the chain wil try to contract
    if(hasToken<chain_ContractToken>()){
        chain_handleContractToken();
    }
}

void GSFParticle::chain_handleContractToken()
{
    Q_ASSERT(_state == State::CHAIN_FOLLOWER);
    auto token = peekAtToken<chain_ContractToken>();
    if(isExpanded()){
        if(!(_level == _depth)){
            if(token->_final){
                for(int label : tailLabels()){
                    if(hasNbrAtLabel(label) && nbrAtLabel(label)._level == _level &&
                            nbrAtLabel(label)._depth == _depth+1){
                        if(canPull(label)){
                            nbrAtLabel(label)._ldrlabel =
                                    dirToNbrDir(nbrAtLabel(label), (tailDir() + 3) % 6);
                            pull(label);
                            token = takeToken<chain_ContractToken>();
                        } else {
                            nbrAtLabel(label).putToken(token);
                        }
                        break;
                    }
                }
            } else {
                for(int label : tailLabels()){
                    if(hasNbrAtLabel(label) && nbrAtLabel(label)._level == _level &&
                            nbrAtLabel(label)._depth == _depth+1){
                        if(canPull(label)){
                            nbrAtLabel(label)._ldrlabel =
                                    dirToNbrDir(nbrAtLabel(label), (tailDir() + 3) % 6);
                            pull(label);
                            token = takeToken<chain_ContractToken>();
                            _sent_pull = false;
                        } else {
                            if(!_sent_pull){
                                nbrAtLabel(label).putToken(token);
                                _sent_pull = true;
                            }
                        }
                        break;
                    }
                }
            }
        } else {
            token = takeToken<chain_ContractToken>();
            contractTail();
            if(token->_final){
                if(isContracted()){
                    Q_ASSERT(hasNbrAtLabel(_ldrlabel));
                    auto t = std::make_shared<chain_ConfirmContractToken>();
                    nbrAtLabel(_ldrlabel).putToken(t);
                }
            }
        }
    } else {
        if(!(_level == _depth)){
            for(int label : headLabels()){
                if(hasNbrAtLabel(label) && nbrAtLabel(label)._level == _level &&
                        nbrAtLabel(label)._depth == _depth+1){
                    token = takeToken<chain_ContractToken>();
                    nbrAtLabel(label).putToken(token);
                }
            }
        } else {
            token = takeToken<chain_ContractToken>();
            if(token->_final){
                if(isContracted()){
                    Q_ASSERT(hasNbrAtLabel(_ldrlabel));
                    auto t = std::make_shared<chain_ConfirmContractToken>();
                    nbrAtLabel(_ldrlabel).putToken(t);
                }
            }
        }
    }
}

// FOR DEBUG PURPOSES ONLY
void GSFParticle::chain_handleMovementInitToken()
{
    auto token = takeToken<chain_MovementInitToken>();

    //pass token to next particle allong the side of the triangle if necissary
    //and put chaintoken on this particle
    if(hasNbrAtLabel(token->_dirpassed) && token->_lifetime>0){
        token->_lifetime--;
        int dir = token->_dirpassed;
        nbrAtLabel(dir).putToken(token);
    }

    // Make a chain token, which initiates the chain movement
    auto chainToken = std::make_shared<chain_ChainToken>();
    chainToken->_contract = token->_contract;
    chainToken->L = token->L;


    putToken(chainToken);
    _ldrlabel = -1;

    //send a token to followers with their respective depth value
    if(_state != State::COORDINATOR){
        _state = State::CHAIN_COORDINATOR;
        int followerDir = hasNbrAtLabel((token->_dirpassed + 2) % 6) ? (token->_dirpassed + 2) % 6 : (token->_dirpassed + 4) % 6;

        auto t = std::make_shared<chain_DepthToken>();
        t->_passeddir = followerDir;
        t->_depth = 1;
        nbrAtLabel(followerDir).putToken(t);
    }
}

void GSFParticle::chain_handleDepthToken()
{
    Q_ASSERT(_state == State::CHAIN_FOLLOWER);
    auto token = takeToken<chain_DepthToken>();
    _ldrlabel = (token->_passeddir+3)%6;
    _depth = token->_depth;
    token->_depth++;
    if(hasNbrAtLabel(token->_passeddir)){
        nbrAtLabel(token->_passeddir).putToken(token);
    }
}

void GSFParticle::chain_handleConfirmContractToken()
{
    auto token = takeToken<chain_ConfirmContractToken>();
    if(_state == State::CHAIN_FOLLOWER){
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


void GSFParticle::chain_handleChainToken(){
    Q_ASSERT(_state == State::CHAIN_COORDINATOR || _state == State::COORDINATOR);
    auto token = peekAtToken<chain_ChainToken>();
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
                    auto token = takeToken<chain_ChainToken>();
                } else {
                    for(int label : headLabels()){
                        if(hasNbrAtLabel(label) && nbrAtLabel(label)._level == _level&&
                                nbrAtLabel(label)._depth == _depth+1){
                            auto t = std::make_shared<chain_ContractToken>();
                            t->_final = true;
                            nbrAtLabel(label).putToken(t);

                            break;
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
                            auto t = std::make_shared<chain_ContractToken>();
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
                    auto token = takeToken<chain_ChainToken>();
                } else {
                    for(int label : tailLabels()){
                        qDebug() << " label " << label ;
                        if(hasNbrAtLabel(label) && nbrAtLabel(label)._level == _level&&
                                nbrAtLabel(label)._depth == _depth+1){
                            qDebug() << " neigbour at: " << isContracted() ;
                            if(canPull(label)){
                                auto t = std::make_shared<chain_ContractToken>();
                                t->_final = true;
                                nbrAtLabel(label).putToken(t);

                                nbrAtLabel(label)._ldrlabel =
                                            dirToNbrDir(nbrAtLabel(label), (tailDir() + 3) % 6);
                                pull(label);
                            } else {
                                auto t = std::make_shared<chain_ContractToken>();
                                t->_final = true;
                                nbrAtLabel(label).putToken(t);
                            }
                            break;
                        }
                    }
                }
            //otherwise stop chain execution
            } else {
                if(_state != State::COORDINATOR){
                    auto token = takeToken<chain_ChainToken>();
                    _state = State::CHAIN_FOLLOWER;
                    _ldrlabel = -1;
                } else {
                    auto token = takeToken<chain_ChainToken>();
                    _ldrlabel = -1;
                }
            }
        }
    }
    if(hasToken<chain_ConfirmContractToken>()){
        auto token = takeToken<chain_ChainToken>();
        auto token2 = takeToken<chain_ConfirmContractToken>();
        _state = State::CHAIN_FOLLOWER;
        _ldrlabel = -1;
    }
}
