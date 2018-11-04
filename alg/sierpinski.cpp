#include <set>

#include <QtGlobal>

#include "alg/sierpinski.h"
#include <QDebug>

SierpinskiParticle::SierpinskiParticle(const Node head,
                                       const int globalTailDir,
                                       const int orientation,
                                       AmoebotSystem& system,
                                       State state)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      state(state),
      constructionDir(-1),
      moveDir(-1),
      followDir(-1),
      constructionDir2(-1),
      sideDir(-1),
      fractalPoint1(-1),
      fractalPoint2(-1)
{
    if(state == State::Seed) {
        constructionDir = 0;
    }
}

void SierpinskiParticle::activate()
{
    if(isExpanded()) {
        if(state == State::Follow) {
            if(!hasNeighborInState({State::Idle}) && !hasTailFollower()) {
                contractTail();
            }
            return;
        } else if(state == State::Lead) {
            if(!hasNeighborInState({State::Idle}) && !hasTailFollower()) {
                contractTail();
                updateMoveDir();
            }
            return;
        } else {
            Q_ASSERT(false);
        }
    } else {
        if(state == State::Seed) {
            if(fractalPoint1==-1 )//do both at once
            {
                int dir1 = constructionDir;
                int dir2 = (constructionDir+1)%6;
                if(hasNbrAtLabel(dir1) && nbrAtLabel(dir1).state==State::Wait &&
                        hasNbrAtLabel(dir2) && nbrAtLabel(dir2).state==State::Wait)
                {
                    fractalPoint1 = dir1;
                    fractalPoint2 = dir2;
                }
            }

            return;
        } else if(state == State::Idle) {
            if(hasNeighborInState({State::Seed, State::Wait})) {
                state = State::Lead;
                updateMoveDir();
                return;
            } else if(hasNeighborInState({State::Lead, State::Follow})) {
                state = State::Follow;
                followDir = labelOfFirstNeighborInState({State::Lead, State::Follow});
                return;
            }
        } else if(state == State::Follow) {
            if(hasNeighborInState({State::Seed, State::Wait})) {
                state = State::Lead;
                updateMoveDir();
                return;
            } else if(hasTailAtLabel(followDir)) {
                auto neighbor = nbrAtLabel(followDir);
                int neighborContractionDir = nbrDirToDir(neighbor, (neighbor.tailDir() + 3) % 6);
                push(followDir);
                followDir = neighborContractionDir;
                return;
            }
        } else if(state == State::Lead) {
            if(canFinish()) {
                state = State::Wait;
                updateConstructionDir();
                qDebug()<<"# fractal pointers: "<<numFractalPointers();
                return;
            } else {
                updateMoveDir();
                if(!hasNbrAtLabel(moveDir)) {
                    expand(moveDir);
                } else if(hasTailAtLabel(moveDir)) {
                    push(moveDir);
                }
                return;
            }
        }
        else if (state == State::Wait){
            if(fractalPoint1==-1){
                auto fractal1prop = [&](const SierpinskiParticle& p) {
                    return  isContracted() &&
                            (p.state == State::Seed || p.state == State::Wait) &&
                            p.fractalPoint1>-1 && pointsAtMe(p, p.fractalPoint1);
                };
                auto fractal2prop = [&](const SierpinskiParticle& p) {
                    return  isContracted() &&
                            (p.state == State::Seed || p.state == State::Wait) &&
                            p.fractalPoint2>-1 && pointsAtMe(p, p.fractalPoint2);
                };
                for(int label = 0; label<6; label++)
                {
                    if(hasNbrAtLabel(label)){
                        SierpinskiParticle particle = nbrAtLabel(label);
                        if(fractal1prop(particle)) {
                            fractalPoint1 = (label+3)%6;
                        }
                        if(fractal2prop(particle)){
                            fractalPoint2 = (label+3)%6;
                        }
                    }
                }
                if(fractalPoint1>-1 && fractalPoint2==-1){
                    fractalPoint2 = (fractalPoint1+1)%6;
                }
                if(fractalPoint2>-1 && fractalPoint1==-1){
                    fractalPoint1= (fractalPoint2+5)%6;
                }
            }
            if(numFractalPointers()==2)
            {
                fractalPoint1= -1;
                fractalPoint2=-1;
            }
        }
    }
}

int SierpinskiParticle::headMarkColor() const
{
    switch(state) {
    case State::Seed:   return 0x00ff00;
    case State::Idle:   return -1;
    case State::Follow: return 0x0000ff;
    case State::Lead:   return 0xff0000;
    case State::Wait:
        if(numFractalPointers()>1)
            return 0x00ff00;
        return 0x000000;
    }

    return -1;
}

int SierpinskiParticle::headMarkDir() const
{
    if(state == State::Lead) {
        return moveDir;
    } else if(state == State::Seed || state == State::Wait) {
        return constructionDir;
    } else if(state == State::Follow) {
        return followDir;
    }
    return -1;
}

int SierpinskiParticle::tailMarkColor() const
{
    return headMarkColor();
}

QString SierpinskiParticle::inspectionText() const
{
    QString text;
    text += "head: (" + QString::number(head.x) + ", " + QString::number(head.y) + ")\n";
    text += "orientation: " + QString::number(orientation) + "\n";
    text += "globalTailDir: " + QString::number(globalTailDir) + "\n";
    text += "state: ";
    text += [this](){
        switch(state) {
        case State::Seed:   return "seed";
        case State::Idle:   return "idle";
        case State::Follow: return "follow";
        case State::Lead:   return "lead";
        case State::Wait: return "finish";
        }
    }();
    text += "\n";
    return text;
}

SierpinskiParticle& SierpinskiParticle::nbrAtLabel(int label) const
{
    return AmoebotParticle::nbrAtLabel<SierpinskiParticle>(label);
}

int SierpinskiParticle::labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel) const
{
    auto propertyCheck = [&](const SierpinskiParticle& p) {
        for(auto state : states) {
            if(p.state == state) {
                return true;
            }
        }
        return false;
    };

    return labelOfFirstNbrWithProperty<SierpinskiParticle>(propertyCheck, startLabel);
}

bool SierpinskiParticle::hasNeighborInState(std::initializer_list<State> states) const
{
    return labelOfFirstNeighborInState(states) != -1;
}

int SierpinskiParticle::constructionReceiveDir() const
{
    auto propertyCheck = [&](const SierpinskiParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Wait) &&
                (p.constructionDir>-1 && (pointsAtMe(p, p.constructionDir))
                 ||(p.constructionDir2>-1 &&  pointsAtMe(p,p.constructionDir2)));
    };

    return labelOfFirstNbrWithProperty<SierpinskiParticle>(propertyCheck);
}
int SierpinskiParticle::sideReceiveDir() const
{
    auto propertyCheck = [&](const SierpinskiParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Wait) &&
                p.sideDir>-1&&  (pointsAtMe(p, p.sideDir) );
    };

    return labelOfFirstNbrWithProperty<SierpinskiParticle>(propertyCheck);
}

bool SierpinskiParticle::canFinish() const
{
    return constructionReceiveDir() != -1;
}
int SierpinskiParticle::numFractalPointers() const
{

    auto fractalprop = [&](const SierpinskiParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Wait) &&
               (( p.fractalPoint1>-1 && pointsAtMe(p, p.fractalPoint1)) ||
                ( p.fractalPoint2>-1 && pointsAtMe(p, p.fractalPoint2)));
    };
    int count = 0;
    for(int label = 0; label<6; label++)
    {
        if(hasNbrAtLabel(label) && fractalprop(nbrAtLabel(label))){
            count++;
        }
    }
    return count;
}



void SierpinskiParticle::updateConstructionDir()
{
    constructionDir = constructionReceiveDir();
    int direction = constructionDir;
    if(hasNbrAtLabel(constructionDir) && nbrAtLabel(constructionDir).state == State::Seed) {
        constructionDir = (constructionDir + 1) % 6;
    }
    if((hasNbrAtLabel((direction+4)%6)&&nbrAtLabel((direction+4)%6).state == State::Wait )
            ||(hasNbrAtLabel((direction+2)%6)&& nbrAtLabel((direction+2)%6).state== State::Wait)){
        constructionDir =(direction+3)%6;
    } else{
        if ((!hasNbrAtLabel((direction+1)%6) || nbrAtLabel((direction+1)%6).state != State::Wait)
                && (!hasNbrAtLabel((direction+5)%6) || nbrAtLabel((direction+5)%6).state!= State::Wait )&&
                (!hasNbrAtLabel((direction+5)%6) || nbrAtLabel((direction+5)%6).state!= State::Seed)){
            if (sideReceiveDir()>-1 || nbrAtLabel(direction).state ==State::Seed){
                constructionDir=(direction+5)%6;
            } else {
                constructionDir=(direction+1)%6;
            }
        } else if (hasNeighborInState({State::Seed}) || (hasNbrAtLabel((direction+5)%6) && nbrAtLabel((direction+5)%6).state== State::Wait)){
            constructionDir =(direction+2)%6;
        } else if ( hasNbrAtLabel((direction+1)%6) && nbrAtLabel((direction+1)%6).state== State::Wait){
            constructionDir=(direction+4)%6;
            constructionDir2=(direction+4)%6;
            sideDir = constructionDir2;
        }
    }

}

void SierpinskiParticle::updateMoveDir()
{
    moveDir = labelOfFirstNeighborInState({State::Seed, State::Wait});
    while(hasNbrAtLabel(moveDir) && (nbrAtLabel(moveDir).state == State::Seed || nbrAtLabel(moveDir).state == State::Wait)) {
        moveDir = (moveDir + 5) % 6;
    }
}

bool SierpinskiParticle::hasTailFollower() const
{
    auto propertyCheck = [&](const SierpinskiParticle& p) {
        return  p.state == State::Follow &&
                pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
    };
    return labelOfFirstNbrWithProperty<SierpinskiParticle>(propertyCheck) != -1;
}

SierpinskiSystem::SierpinskiSystem(int numParticles, double holeProb)
{
    insert(new SierpinskiParticle(Node(0, 0), -1, randDir(), *this, SierpinskiParticle::State::Seed));

    std::set<Node> occupied;
    occupied.insert(Node(0, 0));

    std::set<Node> candidates;
    for(int i = 0; i < 6; i++) {
        candidates.insert(Node(0, 0).nodeInDir(i));
    }

    // add inactive particles
    int numNonStaticParticles = 0;
    while(numNonStaticParticles < numParticles && !candidates.empty()) {
        // pick random candidate
        int randIndex = randInt(0, candidates.size());
        Node randomCandidate;
        for(auto it = candidates.begin(); it != candidates.end(); ++it) {
            if(randIndex == 0) {
                randomCandidate = *it;
                candidates.erase(it);
                break;
            } else {
                randIndex--;
            }
        }

        occupied.insert(randomCandidate);

        if(randBool(1.0 - holeProb)) {
            // only add particle if not a hole
            insert(new SierpinskiParticle(randomCandidate, -1, randDir(), *this, SierpinskiParticle::State::Idle));
            numNonStaticParticles++;

            // add new candidates
            for(int i = 0; i < 6; i++) {
                auto neighbor = randomCandidate.nodeInDir(i);
                if(occupied.find(neighbor) == occupied.end()) {
                    candidates.insert(neighbor);
                }
            }
        }
    }
}

bool SierpinskiSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    if(!isConnected(particles)) {
        return true;
    }
#endif

    for(auto p : particles) {
        auto hp = dynamic_cast<SierpinskiParticle*>(p);
        if(hp->state != SierpinskiParticle::State::Seed && hp->state != SierpinskiParticle::State::Finish)
        {
            return false;
        }
    }

    return true;
}
