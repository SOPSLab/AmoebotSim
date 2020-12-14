#include "generalshapeformation.h"

GSFParticle::GSFParticle(Node& head, const int globalTailDir,
                                   const int orientation, AmoebotSystem& system,
                                   const int sideLen, const State state)
        : AmoebotParticle(head, globalTailDir, orientation, system),
          sideLen(sideLen),
          _state(state){

}

//Implement activate
void GSFParticle::activate(){

}

int GSFParticle::headMarkColor() const{
    switch(_state){
        case State::COORDINATOR: return 0x00ff00;
    }
    return -1;
}

int GSFParticle::tailMarkColor() const{
    headMarkColor();
}

QString GSFParticle::inspectionText() const{
    QString text;
    text += "We're no strangers to looooove";
    return text;
}

GSFSystem::GSFSystem(int sideLen){
    int dir  = 4;
    std::set<Node> occupied;
    Node current(0,0);
    auto coordinator = new GSFParticle(current, -1, randDir(), *this, sideLen,
                                        GSFParticle::State::COORDINATOR);
    insert(coordinator);
    current = current.nodeInDir(dir%6);
    for(int i = 1; i < sideLen; i++){
        auto sideP = new GSFParticle(current, -1, randDir(), *this, sideLen,
                                          GSFParticle::State::FOLLOWER);
        insert(sideP);
        Node temp = current.nodeInDir((dir+2)%6);
        for(int j = 0; j<i; j++){

            auto innerP = new GSFParticle(temp, -1, randDir(), *this, sideLen,
                                              GSFParticle::State::FOLLOWER);
            insert(innerP);
            temp = temp.nodeInDir((dir+2)%6);
        }
        current = current.nodeInDir(dir%6);
    }
}
