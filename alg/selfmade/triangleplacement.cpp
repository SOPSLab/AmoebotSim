/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */


#include "alg/selfmade/triangleplacement.h"

TriangleParticle::TriangleParticle(Node& head, const int globalTailDir,
                                   const int orientation, AmoebotSystem& system,
                                   const int sideLen, const State state)
        : AmoebotParticle(head, globalTailDir, orientation, system),
          sideLen(sideLen),
          _state(state){

}

//Implement activate
void TriangleParticle::activate(){

}

int TriangleParticle::headMarkColor() const{
    switch(_state){
        case State::COORDINATOR: return 0x00ff00;
    }
    return -1;
}

int TriangleParticle::tailMarkColor() const{
    headMarkColor();
}

QString TriangleParticle::inspectionText() const{
    QString text;
    text += "We're no strangers to looooove";
    return text;
}

TriangleSystem::TriangleSystem(int sideLen){
    int dir  = 4;
    std::set<Node> occupied;
    Node current(0,0);
    auto coordinator = new TriangleParticle(current, -1, randDir(), *this, sideLen,
                                        TriangleParticle::State::COORDINATOR);
    insert(coordinator);
    current = current.nodeInDir(dir%6);
    for(int i = 1; i < sideLen; i++){
        auto sideP = new TriangleParticle(current, -1, randDir(), *this, sideLen,
                                          TriangleParticle::State::FOLLOWER);
        insert(sideP);
        Node temp = current.nodeInDir((dir+2)%6);
        for(int j = 0; j<i; j++){

            auto innerP = new TriangleParticle(temp, -1, randDir(), *this, sideLen,
                                              TriangleParticle::State::FOLLOWER);
            insert(innerP);
            temp = temp.nodeInDir((dir+2)%6);
        }
        current = current.nodeInDir(dir%6);
    }
}
