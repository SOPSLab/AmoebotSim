#include <set>

#include <QtGlobal>

#include "alg/matrix2.h"

#include <QDebug>
Matrix2Particle::Matrix2Particle(const Node head,
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
      setlocValue(false),
      resultValue(-1),
      sentProduct(false)
{

}
void Matrix2Particle::setCounterGoal(int goal){

}
void Matrix2Particle::activate()
{

    if(state == State::Seed) {
        return;
    } else if(state == State::Vector) {
        if(!setlocValue){
            locationValue = 1;//rand() % 10 + 1;
            setlocValue = true;
            std::shared_ptr<VectorToken> vtoken= std::make_shared<VectorToken>();
            vtoken->value = locationValue;
            followDir = labelOfFirstNeighborInState({State::Matrix},0);
            neighborAtLabel(followDir).putToken(vtoken);
            state=State::Finish;

        }


    }
    else if(state == State::Matrix) {
        if(!setlocValue){
            locationValue =1 ;//rand() % 10 + 1;
            setlocValue = true;
        }
        if(followDir<0)
        {
            auto propertyCheck = [&](const Matrix2Particle& p) {
                return  p.followDir!=-1&&  pointsAtMe(p, p.followDir);
            };
            int followPoint = labelOfFirstNeighborWithProperty<Matrix2Particle>(propertyCheck);
            if(followPoint!= -1)
            {
                followDir =(followPoint+3)%6;
            }
        }
        if(followDir!=-1 && countTokens<VectorToken>()>0)//require follow dir here so handling tokens done in 1 step, no waiting
        {
            //do multiplication
            std::shared_ptr<VectorToken> vtoken = takeToken<VectorToken>();

            /* //make product
            std::shared_ptr<ProductToken> ptoken = std::make_shared<ProductToken>();
            ptoken->value = locationValue*vtoken->value;
            //TODO: not if neighbor alread has a ptoken!

            neighborAtLabel((followDir+5)%6).putToken(ptoken);*/
            int product = locationValue*vtoken->value;
            for(int i =0; i<product; i++)
            {
                putToken(std::make_shared<SumToken>());
            }
            sentProduct = true;

            //send forward if possible
            if( hasNeighborAtLabel(followDir))
            {
                neighborAtLabel(followDir).putToken(vtoken);

            }

        }
        /* if(countTokens<ProductToken>()>0){
            neighborAtLabel((followDir+5)%6).putToken(takeToken<ProductToken>());
        }*/
        //pass as much as possible
      if(followDir!=-1)
      {
          while(neighborAtLabel((followDir+5)%6).countTokens<SumToken>()<tokenMax && countTokens<SumToken>()>0){
            neighborAtLabel((followDir+5)%6).putToken(takeToken<SumToken>());
             qDebug()<<"pass to: "<<((followDir+5)%6);
          }
        }
        if(followDir!=-1&& !hasNeighborAtLabel((followDir+1)%6) && !hasNeighborAtLabel((followDir+2)%6)
                && countTokens<SumToken>()==0 && sentProduct){
            state = State::Finish;
        }
        else if(followDir!=-1 && hasNeighborAtLabel((followDir+2)%6)  && neighborAtLabel((followDir+2)%6).state==State::Finish
                && countTokens<SumToken>()==0){
            state=State::Finish;
        }
    }

    else if (state == State::Result){
        if(followDir<0)
        {

            auto propertyCheck = [&](const Matrix2Particle& p) {
                return  (p.followDir!=-1&&  pointsAtMe(p, p.followDir)) || p.state==State::Matrix;
            };
            int followPoint = labelOfFirstNeighborWithProperty<Matrix2Particle>(propertyCheck);
          //  qDebug()<<"followPoint: "<<followPoint;
            if(followPoint!= -1)
            {
                //if there are two matrix neighbors, actually go from second.
                if(neighborAtLabel(followPoint).state==State::Matrix &&
                        hasNeighborAtLabel((followPoint+1)%6) && neighborAtLabel((followPoint+1)%6).state==State::Matrix)
                    followPoint = (followPoint+1)%6;
                followDir =(followPoint+3)%6;
            }

        }
        if(followDir!=-1)
        {
            //carryover if needed
            if(countTokens<SumToken>() ==tokenMax && neighborAtLabel(followDir).countTokens<SumToken>()<tokenMax)
            {
                qDebug()<<"results carryover: "<<countTokens<SumToken>();
                //pass 1 (carryover)
                neighborAtLabel(followDir).putToken(takeToken<SumToken>());
                //discard the rest
                int discardcount = 0;
                while(countTokens<SumToken>()>0){
                    discardcount ++;
                    takeToken<SumToken>();
                }
                qDebug()<<"discarded: "<<discardcount;
            }
        }
        /*  if(countTokens<ProductToken>()>0){
            std::shared_ptr<ProductToken> ptoken = takeToken<ProductToken>();
            int moveAmount = std::min(ptoken->value,MaxValue - resultValue);
            ptoken->value = ptoken->value - moveAmount;
            resultValue = resultValue + moveAmount;
            if(ptoken->value>0){
                neighborAtLabel(followDir).putToken(ptoken);
            }

        }*/
        if(followDir!=-1 && hasNeighborAtLabel((followDir+3)%6)  && neighborAtLabel((followDir+3)%6).state==State::Finish
                && countTokens<SumToken>()<tokenMax){
                   state=State::Finish;
               }
    }
    else{
        state=State::Finish;
    }




}
bool Matrix2Particle::hasSpace()
{
    if(countTokens<VectorToken>()<2)
        return true;

    return false;
}



int Matrix2Particle::headMarkColor() const
{
    switch(state) {
    case State::Seed:   return 0x00ff00;
    case State::Idle:   return -1;
    case State::Follow: return 0x0000ff;
    case State::Lead:   return 0xff0000;
    case State::Finish:
        if(countTokens<SumToken>()>=tokenMax)
        return 0xff0000;
        else if (countTokens<SumToken>()>0)
            return 0x999999;
        return 0x000000;
    case State::Active: return 0x0000ff;
    case State::Matrix:
        if(countTokens<VectorToken>()>0)
            return 0x006400;
        else if (countTokens<SumToken>()>0)
            return 0x002d00;
        return 0x00ff00;
    case State::Result:
        if (countTokens<SumToken>()>0)
                    return 0xff0000;
        return 0x0000ff;
    case State::Vector:
        return 0xff0000;
    }

    return -1;
}

int Matrix2Particle::headMarkDir() const
{
    if(state == State::Lead) {
        return moveDir;
    } else if(state == State::Seed || state == State::Finish) {
        return constructionDir;
    } else if(state == State::Follow || state== State::Active || state ==State::Matrix || state == State::Vector || state == State::Result) {
        return followDir;
    }
    return -1;
}

int Matrix2Particle::tailMarkColor() const
{
    return headMarkColor();
}

QString Matrix2Particle::inspectionText() const
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

Matrix2Particle& Matrix2Particle::neighborAtLabel(int label) const
{
    return AmoebotParticle::neighborAtLabel<Matrix2Particle>(label);
}

int Matrix2Particle::labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel) const
{
    auto propertyCheck = [&](const Matrix2Particle& p) {
        for(auto state : states) {
            if(p.state == state) {
                return true;
            }
        }
        return false;
    };

    return labelOfFirstNeighborWithProperty<Matrix2Particle>(propertyCheck, startLabel);
}

bool Matrix2Particle::hasNeighborInState(std::initializer_list<State> states) const
{
    return labelOfFirstNeighborInState(states) != -1;
}

int Matrix2Particle::constructionReceiveDir() const
{
    auto propertyCheck = [&](const Matrix2Particle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Finish) &&
                pointsAtMe(p, p.constructionDir);
    };

    return labelOfFirstNeighborWithProperty<Matrix2Particle>(propertyCheck);
}

bool Matrix2Particle::canFinish() const
{
    return constructionReceiveDir() != -1;
}

void Matrix2Particle::updateConstructionDir()
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

void Matrix2Particle::updateMoveDir()
{
    moveDir = labelOfFirstNeighborInState({State::Seed, State::Finish});
    while(hasNeighborAtLabel(moveDir) && (neighborAtLabel(moveDir).state == State::Seed || neighborAtLabel(moveDir).state == State::Finish)) {
        moveDir = (moveDir + 5) % 6;
    }
}

bool Matrix2Particle::hasTailFollower() const
{
    auto propertyCheck = [&](const Matrix2Particle& p) {
        return  p.state == State::Follow &&
                pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
    };
    return labelOfFirstNeighborWithProperty<Matrix2Particle>(propertyCheck) != -1;
}

Matrix2System::Matrix2System(int numParticles, int countValue)
{
    // numParticles = 11;
    Matrix2Particle *newparticle = new Matrix2Particle(Node(0, 0), -1, randDir(), *this, Matrix2Particle::State::Seed);
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
        insert(new Matrix2Particle(randomCandidate, -1,1, *this, Matrix2Particle::State::Vector));
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
            insert(new Matrix2Particle(randomCandidate2, -1,2, *this, Matrix2Particle::State::Matrix));
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
        insert(new Matrix2Particle(randomCandidate, -1,1, *this, Matrix2Particle::State::Idle));
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
            insert(new Matrix2Particle(randomCandidate2, -1,2, *this, Matrix2Particle::State::Result));
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

bool Matrix2System::hasTerminated() const
{
#ifdef QT_DEBUG
    if(!isConnected(particles)) {
        return true;
    }
#endif

    for(auto p : particles) {
        auto hp = dynamic_cast<Matrix2Particle*>(p);
        if(hp->state != Matrix2Particle::State::Seed && hp->state != Matrix2Particle::State::Finish) {
            return false;
        }
    }

    return true;
}
