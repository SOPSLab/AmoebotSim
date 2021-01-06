#include "debugparticle.h"
#include "qdebug.h"

DebugParticle::DebugParticle(const Node& head, const int globalTailDir,
                             const int orientation,
                             AmoebotSystem& system, int leaderLabel, int id)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      _leaderLabel(leaderLabel), _id(id){};


void DebugParticle::activate() {
    if(isContracted()){
        if(canExpand(1) && !hasNbrAtLabel(_leaderLabel)){
            expand(1);
            _leaderLabel = dirToHeadLabel(1);
        }
    }else{
        // Store the orientation of my particle (direction from head to tail).
        int initialTailDir = tailDir();

        // Find a neighbor that points at my tail, and handover.
        // Update the leader dir of my neighbor in the process
        bool performedHandover = false;
        for(int i = 0; i < 6; i++){
            int iToTailLabel = dirToTailLabel(i);
            if(hasNbrAtLabel(iToTailLabel) && canPull(iToTailLabel)){
                // We are going to contract, so set the label to the actual dir
                _leaderLabel = labelToDir(_leaderLabel);

                // Handover (pull), and update leaderLabel of nbr, to the opposite direction of the initialTailDir
                auto nbr = nbrAtLabel(iToTailLabel);

                int newNeighborLeaderLabel = nbr.dirToHeadLabelAfterExpansion((initialTailDir + 3) % 6, (i+3) % 6);

                nbrAtLabel(iToTailLabel)._leaderLabel = newNeighborLeaderLabel;

                pull(iToTailLabel);
                performedHandover = true;
                break;
            }else if(hasNbrAtLabel(iToTailLabel) && !canPull(iToTailLabel)){
                // Dirty hack to prevent contractions in the middle of a chain. Can be removed in final implementation
                performedHandover = true;
                break;
            }
        }

        // If no neighbor was found, then I'm last in the chain, so just contract tail.
        // This also needs to be polished in final implementation
        if(!performedHandover){
            _leaderLabel = labelToDir(_leaderLabel);
            contractTail();
        }
    }
}

DebugParticle& DebugParticle::nbrAtLabel(int label) const{
    return AmoebotParticle::nbrAtLabel<DebugParticle>(label);
}

DebugSystem::DebugSystem(){
    Node current(0,0);
    for(int i = 0; i < 5; i++){
        auto p = new DebugParticle(current, -1, 0, *this, 3, i);
        insert(p);
        current = current.nodeInDir(0);
    }
}

QString DebugParticle::inspectionText() const{
    qDebug() << "inspecting...";
    qDebug() << _leaderLabel;
    QString text;
    text = "Hoi\nLeaderLabel: " + QString::number(_leaderLabel) + "\n";
    text += "tailDir: " + QString::number(tailDir()) + "\n";
    text += "dirToHeadLabel(3) => " + QString::number(dirToHeadLabel(3)) + "\n";
    text += "dirToTailLabel(3) => " + QString::number(dirToTailLabel(3)) + "\n";
    if(hasNbrAtLabel(0)){
        auto nbr = nbrAtLabel(0);

        for(int i = 0; i < 10; i++){
            if(pointsAtMe(nbr, i)){
                text += "Rechterbuurman points at me from " + QString::number(i) + "\n";
            }
        }


    }
    return text;
}

int DebugParticle::headMarkDir() const{
    if(isContracted()){
        return _leaderLabel;
    }else{
        return labelToDir(_leaderLabel);
    }
}

int DebugParticle::tailMarkDir() const{
    if(isContracted()){
        return -1;
    }else{
        return -1;
    }
}

int DebugParticle::headMarkColor() const{
    return 0x332211;
}

int DebugParticle::tailMarkColor() const{
    return 0x332211;
}
