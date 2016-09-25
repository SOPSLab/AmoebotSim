#include <set>

#include <QtGlobal>

#include "alg/linesort.h"
#include <QDebug>

LineSortParticle::LineSortParticle(const Node head,
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
      value(-1),
      insertDir(-1)

{
    if(state == State::Seed) {
        constructionDir = 0;
    }
}

void LineSortParticle::activate()
{
    if(value ==-1)
    {
        value = rand() % maxValue + 1;
        qDebug()<<"value: "<<value;
    }
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
            constructionDir = 0;
            constructionDir2 = 3;
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
                auto neighbor = neighborAtLabel(followDir);
                int neighborContractionDir = neighborDirToDir(neighbor, (neighbor.tailDir() + 3) % 6);
                push(followDir);
                followDir = neighborContractionDir;
                return;
            }
        } else if(state == State::Lead) {
            if(canWait()) {
                state = State::Wait;
                updateConstructionDir();
                return;
            }
            else if (canInsert())
            {
                state = State::Wait;
                return;
            }
            else {
                updateMoveDir();
                if(!hasNeighborAtLabel(moveDir)) {
                    expand(moveDir);
                } else if(hasTailAtLabel(moveDir)) {
                    push(moveDir);
                }
                return;
            }
        }
        else if (state == State::Wait){

        }
    }
}

int LineSortParticle::valueToHex(int colorValue) const
{
    int gValue = (int)(((double)colorValue)/((double)maxValue)*255);
    int rValue = 150;
    int bValue = 150;

    return rValue << 16 | gValue << 8 | bValue;
}
int LineSortParticle::headMarkColor() const
{

    switch(state) {
    case State::Seed:   return 0x000000;
    case State::Idle:   return -1;
    case State::Follow: return 0x0000ff;
    case State::Lead:   return 0xff0000;
    case State::Wait:
        if(countTokens<ComplaintToken>()>0)
                return 0x0000ff;
        return valueToHex(value);


    }

    return -1;
}

int LineSortParticle::headMarkDir() const
{
    if(state == State::Lead) {
        return moveDir;
    } else if(state == State::Seed || state == State::Wait) {
        //constructionDir or opposite of Dir2 should mean all go same way
        if(constructionDir>-1)
            return constructionDir;
        else
            return (constructionDir2+3)%6;
    } else if(state == State::Follow) {
        return followDir;
    }
    return -1;
}

int LineSortParticle::tailMarkColor() const
{
    return headMarkColor();
}

QString LineSortParticle::inspectionText() const
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

LineSortParticle& LineSortParticle::neighborAtLabel(int label) const
{
    return AmoebotParticle::neighborAtLabel<LineSortParticle>(label);
}

int LineSortParticle::labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel) const
{
    auto propertyCheck = [&](const LineSortParticle& p) {
        for(auto state : states) {
            if(p.state == state) {
                return true;
            }
        }
        return false;
    };

    return labelOfFirstNeighborWithProperty<LineSortParticle>(propertyCheck, startLabel);
}

bool LineSortParticle::hasNeighborInState(std::initializer_list<State> states) const
{
    return labelOfFirstNeighborInState(states) != -1;
}

int LineSortParticle::constructionReceiveDir1() const
{
    auto propertyCheck = [&](const LineSortParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Wait) &&
                (p.constructionDir>-1 && (pointsAtMe(p, p.constructionDir)));
    };

    return labelOfFirstNeighborWithProperty<LineSortParticle>(propertyCheck);
}

int LineSortParticle::constructionReceiveDir2() const
{
    auto propertyCheck = [&](const LineSortParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Wait) &&
                (p.constructionDir2>-1 &&  pointsAtMe(p,p.constructionDir2));
    };

    return labelOfFirstNeighborWithProperty<LineSortParticle>(propertyCheck);
}

bool LineSortParticle::canWait() const
{
    int  constructionRecDir1 = constructionReceiveDir1();
    if(constructionRecDir1!=-1)
    {
        if(value>neighborAtLabel(constructionRecDir1).value)
            return true;
    }
    int constructionRecDir2 = constructionReceiveDir2();
    if(constructionRecDir2!=-1)
    {
        if(value<neighborAtLabel(constructionRecDir2).value)
            return true;
    }
    return false;
}
bool LineSortParticle::canInsert() const
{
    //otherwise see if stopping on line between to wait for an opening
    int firstNeighbor = labelOfFirstNeighborInState({State::Seed, State::Wait});
    if(firstNeighbor>=0)
    {
        int neighbor1Value = neighborAtLabel(firstNeighbor).value;
        int secondNeighbor=-1;
        for(int i =1; i<6; i++)//check other 5 directions
        {
            if(hasNeighborAtLabel((firstNeighbor+i)%6) && (neighborAtLabel((firstNeighbor+i)%6).state==State::Seed
                                                           || neighborAtLabel((firstNeighbor+i)%6).state==State::Wait))
            {
                secondNeighbor=(firstNeighbor+i)%6;
                int neighbor2Value = neighborAtLabel(secondNeighbor).value;

                //somehow between neighbors- save as bool for later use
                bool n2HigherBetween = (value>=neighbor1Value && value<=neighbor2Value);
                bool n1HigherBetween = (value>=neighbor2Value && value<=neighbor1Value);
                if(n1HigherBetween || n2HigherBetween)
                {
                    //check if both neighbors actually on line
                    if((neighborAtLabel(firstNeighbor).constructionDir!=-1 || neighborAtLabel(firstNeighbor).constructionDir2!=-1)&&
                            (neighborAtLabel(secondNeighbor).constructionDir!=-1 || neighborAtLabel(secondNeighbor).constructionDir2!=-1))
                    {
                        std::shared_ptr<ComplaintToken> ctoken= std::make_shared<ComplaintToken>();
                        int complaintRecepient = -1;
                        //case 0: one is a seed, complain to the wait
                        if(neighborAtLabel(firstNeighbor).state==State::Seed){ complaintRecepient = secondNeighbor;}
                        else if(neighborAtLabel(secondNeighbor).state==State::Seed){ complaintRecepient = firstNeighbor;}

                        //case 1: both have constructionDir2, so complain to the lower value to make space
                        else if(neighborAtLabel(firstNeighbor).constructionDir2!=-1 && neighborAtLabel(secondNeighbor).constructionDir2!=-1){
                            if(n1HigherBetween){ complaintRecepient = secondNeighbor;}
                            else if(n2HigherBetween){ complaintRecepient = firstNeighbor;}
                        }
                        //case 2: both have constructionDir2 so complain to upper value to make space
                        else if(neighborAtLabel(firstNeighbor).constructionDir!=-1 && neighborAtLabel(secondNeighbor).constructionDir!=-1){
                            if(n1HigherBetween){ complaintRecepient = firstNeighbor;}
                            else if(n2HigherBetween){ complaintRecepient = secondNeighbor;}
                        }
                        if(complaintRecepient!=-1)
                        {
                            neighborAtLabel(complaintRecepient).putToken(ctoken);
                        }
                        else
                        {
                            qDebug()<<"no complaint recepient!";
                        }
                        return true;
                    }
                }
            }
        }
    }

    return false;
    // return (constructionReceiveDir1() != -1 || constructionReceiveDir2()!=-1);
}


void LineSortParticle::updateConstructionDir()
{
    int cdir1 = constructionReceiveDir1();
    int  cdir2 =constructionReceiveDir2();
    if(cdir1>-1 && cdir1<6)
        constructionDir = (cdir1+3)%6;
    else if (cdir2>-1 && cdir2<6)
        constructionDir2 =(cdir2+3)%6;


}

void LineSortParticle::updateMoveDir()
{
    // qDebug()<<"my value: "<<value;
    moveDir = labelOfFirstNeighborInState({State::Seed, State::Wait});
    /* if(moveDir>=0){
        qDebug()<<"neighbor1 value: "<<neighborAtLabel(moveDir).value;
    }
    for(int i =1; i<6; i++)//check other 5 directions
    {
        if(hasNeighborAtLabel((moveDir+i)%6) && (neighborAtLabel((moveDir+i)%6).state==State::Seed || neighborAtLabel((moveDir+i)%6).state==State::Wait)){
            qDebug()<<"neighbor 2 value: "<<neighborAtLabel((moveDir+i)%6).value;
        }
    }*/
    while(hasNeighborAtLabel(moveDir) && (neighborAtLabel(moveDir).state == State::Seed || neighborAtLabel(moveDir).state == State::Wait)) {
        moveDir = (moveDir + 5) % 6;
    }
}

bool LineSortParticle::hasTailFollower() const
{
    auto propertyCheck = [&](const LineSortParticle& p) {
        return  p.state == State::Follow &&
                pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
    };
    return labelOfFirstNeighborWithProperty<LineSortParticle>(propertyCheck) != -1;
}

LineSortSystem::LineSortSystem(int numParticles, float holeProb)
{
    insert(new LineSortParticle(Node(0, 0), -1, randDir(), *this, LineSortParticle::State::Seed));

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

        if(randBool(1.0f - holeProb)) {
            // only add particle if not a hole
            insert(new LineSortParticle(randomCandidate, -1, randDir(), *this, LineSortParticle::State::Idle));
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

bool LineSortSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    if(!isConnected(particles)) {
        return true;
    }
#endif

    for(auto p : particles) {
        auto hp = dynamic_cast<LineSortParticle*>(p);
        if(hp->state != LineSortParticle::State::Seed && hp->state != LineSortParticle::State::Finish)
        {
            return false;
        }
    }

    return true;
}
