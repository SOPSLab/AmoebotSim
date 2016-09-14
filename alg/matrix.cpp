#include <set>

#include <QtGlobal>

#include "alg/matrix.h"

#include <QDebug>
MatrixParticle::MatrixParticle(const Node head,
                               const int globalTailDir,
                               const int orientation,
                               AmoebotSystem& system,
                               State state)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      state(state),
      constructionDir(-1),
      moveDir(-1),
      followDir(-1),
      locationValue(0),
      setlocValue(false)
{

}
void MatrixParticle::setCounterGoal(int goal){

}
void MatrixParticle::activate()
{

    if(state == State::Seed) {

        return;
    } else if(state == State::Vector) {
        if(!setlocValue){
            locationValue = rand() % 10 + 1;
            setlocValue = true;
            std::shared_ptr<VectorToken> vtoken= std::make_shared<VectorToken>();
            vtoken->value = locationValue;
            followDir = labelOfFirstNeighborInState({State::Matrix},0);
            neighborAtLabel(followDir).putToken(vtoken);
        }


    }
    else if(state == State::Matrix) {
            if(!setlocValue){
                locationValue = rand() % 10 + 1;
                setlocValue = true;
            }
            if(followDir<0)
            {
                auto propertyCheck = [&](const MatrixParticle& p) {
                    return  p.followDir!=-1&&  pointsAtMe(p, p.followDir);
                };
                int followPoint = labelOfFirstNeighborWithProperty<MatrixParticle>(propertyCheck);
                if(followPoint!= -1)
                {
                    followDir =(followPoint+3)%6;
                }
            }
            if(followDir!=-1 && countTokens<VectorToken>()>0)//require follow dir here so handling tokens done in 1 step, no waiting
            {
                //do multiplication
                std::shared_ptr<VectorToken> vtoken = takeToken<VectorToken>();

                //make product
                std::shared_ptr<ResultToken> rtoken = std::make_shared<ResultToken>();
                rtoken->value = locationValue*vtoken->value;
                neighborAtLabel((followDir+5)%6).putToken(rtoken);
                //send forward if possible
                if( hasNeighborAtLabel(followDir))
                {
                    neighborAtLabel(followDir).putToken(vtoken);

                }
                else if(!hasNeighborAtLabel((followDir+1)%6)){
                    //send bottom corner acknowledgement?
                }
            }
            //combine, pass along results
            if(countTokens<ResultToken>()>0){
                neighborAtLabel((followDir+5)%6).putToken(takeToken<ResultToken>());
            }


        }




}
bool MatrixParticle::hasSpace()
{
    if(countTokens<VectorToken>()<2)
        return true;

    return false;
}



int MatrixParticle::headMarkColor() const
{
    switch(state) {
    case State::Seed:   return 0x00ff00;
    case State::Idle:   return -1;
    case State::Follow: return 0x0000ff;
    case State::Lead:   return 0xff0000;
    case State::Finish:
  return 0x999999;
    case State::Active: return 0x0000ff;
    case State::Matrix:
        if(countTokens<VectorToken>()>0)
            return 0x006400;
        else if (countTokens<ResultToken>()>0)
            return 0x002d00;
        return 0x00ff00;
    case State::Result:
        return 0x0000ff;
    case State::Vector:
        return 0xff0000;
    }

    return -1;
}

int MatrixParticle::headMarkDir() const
{
    if(state == State::Lead) {
        return moveDir;
    } else if(state == State::Seed || state == State::Finish) {
        return constructionDir;
    } else if(state == State::Follow || state== State::Active || state ==State::Matrix || state == State::Vector) {
        return followDir;
    }
    return -1;
}

int MatrixParticle::tailMarkColor() const
{
    return headMarkColor();
}

QString MatrixParticle::inspectionText() const
{
    qDebug()<<"inspection";
    QString text;
    text += "head: (" + QString::number(head.x) + ", " + QString::number(head.y) + ")\n";
    text += "orientation: " + QString::number(orientation) + "\n";
    text += "prevLabel: " + QString::number(prevLabel) + "\n";
    text += "nextLabel: " + QString::number(nextLabel) + "\n";
    text += "state: ";
    text += [this](){
        switch(state) {
        case State::Seed:   return "seed";
        case State::Idle:   return "idle";
        case State::Active: return "active";
        }
    }();
    text += "\n Vector Token: " + QString::number(countTokens<VectorToken>());
    text += "\nDisplay: "+QString::number(displayVal)+"\n";
    return text;
}

MatrixParticle& MatrixParticle::neighborAtLabel(int label) const
{
    return AmoebotParticle::neighborAtLabel<MatrixParticle>(label);
}

int MatrixParticle::labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel) const
{
    auto propertyCheck = [&](const MatrixParticle& p) {
        for(auto state : states) {
            if(p.state == state) {
                return true;
            }
        }
        return false;
    };

    return labelOfFirstNeighborWithProperty<MatrixParticle>(propertyCheck, startLabel);
}

bool MatrixParticle::hasNeighborInState(std::initializer_list<State> states) const
{
    return labelOfFirstNeighborInState(states) != -1;
}

int MatrixParticle::constructionReceiveDir() const
{
    auto propertyCheck = [&](const MatrixParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Finish) &&
                pointsAtMe(p, p.constructionDir);
    };

    return labelOfFirstNeighborWithProperty<MatrixParticle>(propertyCheck);
}

bool MatrixParticle::canFinish() const
{
    return constructionReceiveDir() != -1;
}

void MatrixParticle::updateConstructionDir()
{
    constructionDir = constructionReceiveDir();
    if(neighborAtLabel(constructionDir).state == State::Seed) {
        constructionDir = (constructionDir + 1) % 6;
    } else {
        constructionDir = (constructionDir + 2) % 6;
    }

    if(hasNeighborAtLabel(constructionDir) && neighborAtLabel(constructionDir).state == State::Finish) {
        constructionDir = (constructionDir + 1) % 6;
    }
}

void MatrixParticle::updateMoveDir()
{
    moveDir = labelOfFirstNeighborInState({State::Seed, State::Finish});
    while(hasNeighborAtLabel(moveDir) && (neighborAtLabel(moveDir).state == State::Seed || neighborAtLabel(moveDir).state == State::Finish)) {
        moveDir = (moveDir + 5) % 6;
    }
}

bool MatrixParticle::hasTailFollower() const
{
    auto propertyCheck = [&](const MatrixParticle& p) {
        return  p.state == State::Follow &&
                pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
    };
    return labelOfFirstNeighborWithProperty<MatrixParticle>(propertyCheck) != -1;
}

MatrixSystem::MatrixSystem(int numParticles, int countValue)
{
    // numParticles = 11;
    MatrixParticle *newparticle = new MatrixParticle(Node(0, 0), -1, randDir(), *this, MatrixParticle::State::Seed);
    newparticle->setCounterGoal(countValue);

    newparticle->index = 0;
    insert(newparticle);

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
        int randIndex = 1;//randInt(0, candidates.size());
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

        // if(randBool(1.0f - holeProb)) {
        // only add particle if not a hole
        insert(new MatrixParticle(randomCandidate, -1,1, *this, MatrixParticle::State::Vector));
        numNonStaticParticles++;

        // add new candidates
        for(int i = 0; i < 6; i++) {
            auto neighbor = randomCandidate.nodeInDir(i);
            if(occupied.find(neighbor) == occupied.end()) {
                candidates.insert(neighbor);
            }
        }
        //matrix body
        randIndex = 0;//randInt(0, candidates.size());
        Node randomCandidate2;
        std::set<Node> candidates2;
        candidates2 = candidates;
        int numNonStaticParticles2 = 0;
        while(numNonStaticParticles2 < numParticles && !candidates2.empty()) {
            for(auto it = candidates2.begin(); it != candidates2.end(); ++it) {
                if(randIndex == 0) {
                    randomCandidate2 = *it;
                    candidates2.erase(it);
                    break;
                } else {
                    randIndex--;
                }
            }

            occupied.insert(randomCandidate2);

            // if(randBool(1.0f - holeProb)) {
            // only add particle if not a hole
            insert(new MatrixParticle(randomCandidate2, -1,2, *this, MatrixParticle::State::Matrix));
            numNonStaticParticles2++;

            // add new candidates
            for(int i = 0; i < 6; i++) {
                auto neighbor = randomCandidate2.nodeInDir(i);
                if(occupied.find(neighbor) == occupied.end()) {
                    candidates2.insert(neighbor);
                }
            }
        }


    }


    // add inactive particles
     numNonStaticParticles = 0;
    while(numNonStaticParticles < numParticles && !candidates.empty()) {
        // pick random candidate
        int randIndex = 1;//randInt(0, candidates.size());
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

        // if(randBool(1.0f - holeProb)) {
        // only add particle if not a hole
        insert(new MatrixParticle(randomCandidate, -1,1, *this, MatrixParticle::State::Idle));
        numNonStaticParticles++;

        // add new candidates
        for(int i = 0; i < 6; i++) {
            auto neighbor = randomCandidate.nodeInDir(i);
            if(occupied.find(neighbor) == occupied.end()) {
                candidates.insert(neighbor);
            }
        }
        //matrix body
        randIndex = 0;//randInt(0, candidates.size());
        Node randomCandidate2;
        std::set<Node> candidates2;
        candidates2 = candidates;
        int numNonStaticParticles2 = 0;
        while(numNonStaticParticles2 < numParticles && !candidates2.empty()) {
            for(auto it = candidates2.begin(); it != candidates2.end(); ++it) {
                if(randIndex == 0) {
                    randomCandidate2 = *it;
                    candidates2.erase(it);
                    break;
                } else {
                    randIndex--;
                }
            }

            occupied.insert(randomCandidate2);

            // if(randBool(1.0f - holeProb)) {
            // only add particle if not a hole
            insert(new MatrixParticle(randomCandidate2, -1,2, *this, MatrixParticle::State::Result));
            numNonStaticParticles2++;

            // add new candidates
            for(int i = 0; i < 6; i++) {
                auto neighbor = randomCandidate2.nodeInDir(i);
                if(occupied.find(neighbor) == occupied.end()) {
                    candidates2.insert(neighbor);
                }
            }
        }


    }

    // }


}

bool MatrixSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    if(!isConnected(particles)) {
        return true;
    }
#endif

    for(auto p : particles) {
        auto hp = dynamic_cast<MatrixParticle*>(p);
        if(hp->state != MatrixParticle::State::Seed && hp->state != MatrixParticle::State::Finish) {
            return false;
        }
    }

    return true;
}
