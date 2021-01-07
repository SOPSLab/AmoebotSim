#include "../generalshapeformation.h"
#include <QDebug>
#include <iostream>

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

    int followerLabel = -1;


    if(isExpanded()){
        //check if has follower
        for(int label : tailLabels()){
            if(hasNbrAtLabel(label)){
                if(nbrAtLabel(label).hasToken<chain_DepthToken>()){
                    followerLabel = -2;
                    break;
                }
                if(nbrAtLabel(label)._ldrlabel>-1 &&
                        pointsAtMe(nbrAtLabel(label), nbrAtLabel(label).dirToHeadLabel(nbrAtLabel(label)._ldrlabel))){
                    followerLabel = label;
                    qDebug() << "neighbour found at label " << QString::number(followerLabel);
                    break;
                }
            }
        }
        if(followerLabel!= -2){ // check if setup is still in progress
            if(followerLabel>-1){
                auto follower = nbrAtLabel(followerLabel);
                if(token->_final){
                    if(nbrAtLabel(followerLabel).isExpanded()){
                        nbrAtLabel(followerLabel).putToken(token);
                    } else {
                        nbrAtLabel(followerLabel)._ldrlabel =
                                    dirToNbrDir(nbrAtLabel(followerLabel), (tailDir() + 3) % 6);
                        pull(followerLabel);
                        token = takeToken<chain_ContractToken>();
                    }
                } else {
                    auto nbr = nbrAtLabel(followerLabel);
                    if(nbr.isExpanded()){
                        if(!_sent_pull){
                            nbrAtLabel(followerLabel).putToken(token);
                            _sent_pull = true;
                        }

                    } else {
                        nbrAtLabel(followerLabel)._ldrlabel =
                                    dirToNbrDir(nbrAtLabel(followerLabel), (tailDir() + 3) % 6);
                        pull(followerLabel);
                        token = takeToken<chain_ContractToken>();
                        _sent_pull = false;
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
        }
    } else {
        for(int label : headLabels()){
            if(hasNbrAtLabel(label)){
                auto nbr = nbrAtLabel(label);
                if(nbrAtLabel(label).hasToken<chain_DepthToken>()){
                    followerLabel = -2;
                    break;
                }
                if(nbrAtLabel(label)._ldrlabel>-1 &&
                        pointsAtMe(nbrAtLabel(label),
                                   nbrAtLabel(label).dirToHeadLabel(nbrAtLabel(label)._ldrlabel))){
                    followerLabel = label;
                    qDebug() << "neighbour found at label " << QString::number(followerLabel);
                    break;
                }
            }
        }
        if(followerLabel != -2){
            if(followerLabel>-1){
                token = takeToken<chain_ContractToken>();
                nbrAtLabel(followerLabel).putToken(token);
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
}

// FOR DEBUG PURPOSES ONLY
void GSFParticle::chain_handleMovementInitToken()
{
    auto token = takeToken<chain_MovementInitToken>();

    //pass token to next particle along the side of the triangle if necissary
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
        int followerDir = (hasNbrAtLabel((token->_dirpassed + 2) % 6) ? (token->_dirpassed + 2) % 6 : (token->_dirpassed + 4) % 6);
        Q_ASSERT(hasNbrAtLabel(followerDir));

        auto t = std::make_shared<chain_DepthToken>();
        t->_passeddir = followerDir;
        nbrAtLabel(followerDir).putToken(t);
    }
}

void GSFParticle::chain_handleDepthToken()
{
    Q_ASSERT(_state == State::CHAIN_FOLLOWER);
    auto token = takeToken<chain_DepthToken>();
    _ldrlabel = (token->_passeddir+3)%6;
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
                nbrAtLabel(_ldrlabel).putToken(token);
            }
        }
    }
}


void GSFParticle::chain_handleChainToken(){
    Q_ASSERT(_state == State::CHAIN_COORDINATOR || _state == State::COORDINATOR);
    auto token = peekAtToken<chain_ChainToken>();
    int followerLabel = -1;

    //expand to space if the chain coordinator is contracted
    if(isContracted()){
        //find follower
        for(int label : headLabels()){
            if(hasNbrAtLabel(label)){
                auto nbr = nbrAtLabel(label);
                if(nbr.hasToken<chain_DepthToken>()){
                    followerLabel = -2;
                    break;
                }
                if(nbr._ldrlabel>-1 && pointsAtMe(nbr, nbr.dirToHeadLabel(nbr._ldrlabel))){
                    followerLabel = label;
                    break;
                }
            }
        }

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
                    auto t = std::make_shared<chain_ContractToken>();
                    t->_final = true;
                    nbrAtLabel(followerLabel).putToken(t);
                }
            }
        }
    //otherwise pull next node in the chain
    } else {
        for(int label : tailLabels()){
            if(hasNbrAtLabel(label)){
                auto nbr = nbrAtLabel(label);
                if(nbr.hasToken<chain_DepthToken>()){
                    followerLabel = -2;
                    break;
                }
                if(nbr._ldrlabel>-1 && pointsAtMe(nbr, nbr.dirToHeadLabel(nbr._ldrlabel))){
                    followerLabel = label;
                    break;
                }
            }
        }

        if(!token->L.empty()){
            if(followerLabel>-1){
                if(canPull(followerLabel)){
                    auto part = nbrAtLabel(followerLabel);
                    nbrAtLabel(followerLabel)._ldrlabel =
                            dirToNbrDir(nbrAtLabel(followerLabel), (tailDir() + 3) % 6);
                    pull(followerLabel);
                    _sent_pull = false;
                } else {
                    if(!_sent_pull){
                        _sent_pull = true;
                        auto t = std::make_shared<chain_ContractToken>();
                        t->_final = false;
                        nbrAtLabel(followerLabel).putToken(t);
                    }
                }
            }
        } else {
            //Contract the entire chain if so desired
            if (token->_contract){
                if(_state == State::COORDINATOR){
                    contractTail();
                    auto token = takeToken<chain_ChainToken>();
                } else {
                    if(followerLabel>-1){
                        if(canPull(followerLabel)){
                            auto t = std::make_shared<chain_ContractToken>();
                            t->_final = true;
                            nbrAtLabel(followerLabel).putToken(t);

                            nbrAtLabel(followerLabel)._ldrlabel =
                                        dirToNbrDir(nbrAtLabel(followerLabel), (tailDir() + 3) % 6);
                            pull(followerLabel);
                        } else {
                            auto t = std::make_shared<chain_ContractToken>();
                            t->_final = true;
                            nbrAtLabel(followerLabel).putToken(t);
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
                    if(followerLabel>=-1){
                        auto token = takeToken<chain_ChainToken>();
                        _ldrlabel = -1;
                    }
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
