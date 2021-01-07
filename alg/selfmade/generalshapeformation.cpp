#include "generalshapeformation.h"


GSFParticle::GSFParticle(Node& head, const int globalTailDir,
                         const int orientation, AmoebotSystem& system,
                         const int sideLen, int triangleDirection, const State state,
                         unsigned int level, unsigned int ldrlabel,
                         int depth)
        : AmoebotParticle(head, globalTailDir, orientation, system),
          _initialSideLen(sideLen),
          _triangleDirection(triangleDirection),
          _state(state),
          _level(level),
          _ldrlabel(ldrlabel),
          _depth(depth){

}

//Implement activate
void GSFParticle::activate(){
    chain_activate();
    triangle_shift_activate();
    triangle_expand_activate();

}

GSFParticle& GSFParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<GSFParticle>(label);
}


int GSFParticle::headMarkColor() const{
    switch(_state){
        case State::COORDINATOR: return 0x00ff00;
        case State::CHAIN_COORDINATOR: return 0x0000ff;
        case State::CHAIN_FOLLOWER: return 0xff9000;
    }
    return -1;
}

int GSFParticle::headMarkDir() const{
    return _ldrlabel;
}

int GSFParticle::tailMarkColor() const{
    return headMarkColor();
}

QString GSFParticle::inspectionText() const{
    QString text;
    text += "We're no strangers to looooove\n";
    text += " state:";
    text += [this](){
        switch (_state) {
            case State::CHAIN_COORDINATOR: return "chain_coordinator\n";
            case State::COORDINATOR: return "coordinator\n";
            case State::CHAIN_FOLLOWER: return "follower\n";
        }
        return "no state??\n";
    }();
    text += " level: " + QString::number(_level) + "\n";
    text += " depth: " + QString::number(_depth) + "\n";
    text += " triangleDirection: " + QString::number(_triangleDirection) + "\n";

    if(hasToken<chain_ContractToken>()){
        text+= "has contractToken\n";
    }
    if(hasToken<chain_ChainToken>()){
        auto t = peekAtToken<chain_ChainToken>();
        text+= "has chainToken\n";
        if(t->L.empty()){
            text+= "stack L is empty\n";
        } else {
            text+= "stack L is not empty\n";
        }
    }
    if(hasToken<chain_ConfirmContractToken>()){
        text+= "has chainToken\n";
    }
    if (hasToken<triangle_shift_CoordinatorToken>()){
        text += "has shift coordinator token";
    }
    if (hasToken<triangle_shift_TriggerShiftToken>()){
        text += "has trigger shift Token";
    }
    if (hasToken<triangle_shift_CoordinatorToken>()){
        text += "has coordinator shift Token";
    }
    if (hasToken<triangle_shift_ShiftToken>()){
        text += "has shiftToken";
    }
    if(hasToken<chain_DepthToken>()){
        text+= "has depthToken\n";
    }
    if(hasToken<triangle_expand_TriggerExpandToken>()){
        text+= "has triangle_expand_TriggerExpandToken\n";
    }
    if(hasToken<triangle_expand_ConfirmExpandToken>()){
        text+= "has triangle_expand_ConfirmExpandToken\n";
    }else{
        text+= "NOPE!\n";
    }
    if(hasToken<triangle_expand_ExpandToken>()){
        text+= "has a triangle_expand_ExpandToken\n";
    }
    if(hasToken<chain_MovementInitToken>()){
        text+= "has a chain_MovementInitToken\n";
    }
    return text;
}

GSFSystem::GSFSystem(int sideLen, QString expanddir){
    int dir  = 4;
    std::set<Node> occupied;
    Node current(0,0);
    auto coordinator = new GSFParticle(current, -1, 0, *this, sideLen, dir,
                                        GSFParticle::State::COORDINATOR, 0, -1, 0);

    insert(coordinator);

    auto expandToken = std::make_shared<GSFParticle::triangle_expand_TriggerExpandToken>();

    if(expanddir == "l"){
        expandToken->_left = true;
    } else {
        expandToken->_left = false;
    }

    coordinator->putToken(expandToken);


    current = current.nodeInDir(dir%6);
    initializeTriangle(sideLen, current, dir);
}


void GSFSystem::initializeTriangle(int sideLen, Node current, int dir)
{
    for(int i = 1; i < sideLen; i++){
        auto sideP = new GSFParticle(current, -1, 0, *this, sideLen, dir,
                                          GSFParticle::State::CHAIN_FOLLOWER,
                                          i, -1, 0);

        insert(sideP);
        Node temp = current.nodeInDir((dir+2)%6);
        for(int j = 0; j<i; j++){

            auto innerP = new GSFParticle(temp, -1, 0, *this, sideLen, dir,
                                              GSFParticle::State::CHAIN_FOLLOWER,
                                               i, -1, 0);
            insert(innerP);
            temp = temp.nodeInDir((dir+2)%6);
        }
        current = current.nodeInDir(dir%6);
    }
}
