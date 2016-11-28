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
      sentProduct(false),
      streamIter(0),
      sMode(0),
      matrixFlag(-1),
      vectorFlag(-1),
      resultFlag(-1),
      stopReceiveDir(-1),
      columnFinished(false)
{

}
void Matrix2Particle::setCounterGoal(int goal){

}
void Matrix2Particle::activate()
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
        if(countTokens<MatrixToken>() ==0 &&
                countTokens<VectorToken>() ==0 &&
                countTokens<EndOfColumnToken>() ==0 &&
                countTokens<EndOfVectorToken>() ==0 && streamIter<26 )
        {
            if(streamIter==0)
            {
            std::ifstream myfile("/Users/Alex/amoebotsim/alg/matrixstream.txt");
            std::string line;

              if (myfile.is_open())
              {
                while ( getline (myfile,line) )
                {
                  valueStream.push_back(line);
                }
                myfile.close();
              }
            }
              if(valueStream[streamIter].compare("eoc")==0)
              {
                    qDebug()<<"End of column";
                    std::shared_ptr<EndOfColumnToken> eocToken= std::make_shared<EndOfColumnToken>();
                    putToken(eocToken);

              }
              else if(valueStream[streamIter].compare("eov")==0)
              {
                  sMode = 2;
                    qDebug()<<"end of vector";
                   //  std::shared_ptr<StartMultToken> startmult= std::make_shared<StartMultToken>();
                   //    putToken(startmult);
              }
              else if(valueStream[streamIter].compare("eom")==0)
              {
                  sMode = 1;
                    qDebug()<<"end of matrix";
              }
              else
              {
                 int newValue = std::stoi(valueStream[streamIter]);
                 qDebug()<<"new value: "<<newValue;
                 if(sMode ==0)//matrix
                 {
                    std::shared_ptr<MatrixToken> mvToken= std::make_shared<MatrixToken>();
                    mvToken->value = newValue;
                    putToken(mvToken);
                     vectorFlag = 0;

                 }
                 else if (sMode ==1)//vector
                 {
                     std::shared_ptr<VectorToken> vvToken= std::make_shared<VectorToken>();
                     vvToken->value = newValue;
                     putToken(vvToken);

                 }

              }
              streamIter++;


        }
        else if (countTokens<MatrixToken>()>0)
        {
            //continue trying to recruit or if had to wait for space to open up
            if(hasNeighborAtLabel(0) && neighborAtLabel(0).state == State::Vector && noTokensAtLabel(0))
            {

                neighborAtLabel(0).putToken(takeToken<MatrixToken>());
                qDebug()<<"put neighbor matrix token";
            }
        }
        if(countTokens<VectorToken>()>0)
        {
            if(hasNeighborAtLabel(0) && neighborAtLabel(0).state == State::Vector && noTokensAtLabel(0))
            {

                neighborAtLabel(0).putToken(takeToken<VectorToken>());
                qDebug()<<"put neighbor vector token";
            }
        }
        if(countTokens<EndOfColumnToken>()>0 && noTokensAtLabel(0))
        {
            neighborAtLabel(0).putToken(takeToken<EndOfColumnToken>());
            qDebug()<<"put neighbor eoc token";
        }

        return;
    }
    else if(state == State::Idle) {
               if(hasNeighborInState({State::Seed, State::Matrix,State::Vector,State::Result})) {
                   state = State::Lead;
                   updateMoveDir();
                   return;
               } else if(hasNeighborInState({State::Lead, State::Follow})) {
                   state = State::Follow;
                   followDir = labelOfFirstNeighborInState({State::Lead, State::Follow});
                   return;
               }
           }
    else if(state == State::Follow) {
               if(hasNeighborInState({State::Seed, State::Matrix,State::Vector,State::Result})) {
                   state = State::Lead;
                   updateMoveDir();
                   return;
               } else if(hasTailAtLabel(followDir)) {
                   Matrix2Particle neighbor = neighborAtLabel(followDir);
                   int neighborContractionDir = neighborDirToDir(neighbor, (neighbor.tailDir() + 3) % 6);
                   push(followDir);
                   followDir = neighborContractionDir;
                   return;
               }
           }
    else if(state == State::Lead) {
    int vectorStop = tryVectorStop();
    int matrixStop = tryMatrixStop();
    int resultStop = tryResultStop();
               if(vectorStop !=-1) {
                   qDebug()<<"vector stop";
                   state = State::Vector;
                   stopReceiveDir = vectorStop;
                   return;
               }
               else if (matrixStop!=-1){
                   state = State::Matrix;
                   stopReceiveDir = matrixStop;
                   return;
               }
               else if (resultStop!=-1){
                   state = State::Result;
                   stopReceiveDir = resultStop;
                   return;
               }else {
                   updateMoveDir();
                   if(!hasNeighborAtLabel(moveDir)) {
                       expand(moveDir);
                   } else if(hasTailAtLabel(moveDir)) {
                       push(moveDir);
                   }
                   return;
              }
           }
    else if(state == State::Vector) {

        if(countTokens<VectorToken>()>0){
            if(!setlocValue)
            {
                std::shared_ptr<VectorToken> vtoken= takeToken<VectorToken>();
                locationValue = vtoken->value;
                setlocValue = true;
                //pass down column- trigger multiplication
                int matrixDir = (stopReceiveDir+1)%6;
                neighborAtLabel(matrixDir).putToken(vtoken);
            }
            else
            {
                int vectorDir = (stopReceiveDir+3)%6;
                if(noTokensAtLabel(vectorDir))
                {
                    neighborAtLabel(vectorDir).putToken(takeToken<VectorToken>());
                }
            }
        }
        if(countTokens<EndOfColumnToken>()>0)
        {
            if(!columnFinished)
            {
              columnFinished = true;
              takeToken<EndOfColumnToken>();
              qDebug()<<"column finished";
            }
            else
            {
                int vectorDir = (stopReceiveDir+3)%6;
                if(noTokensAtLabel(vectorDir))
                {
                    neighborAtLabel(vectorDir).putToken(takeToken<EndOfColumnToken>());
                }
            }
        }
        if( countTokens<MatrixToken>()>0)
        {
            if(!columnFinished)
            {
                int matrixDir = (stopReceiveDir+1)%6;
                followDir = matrixDir;
                 if(hasNeighborAtLabel(matrixDir) && neighborAtLabel(matrixDir).state == State::Matrix
                    && noTokensAtLabel(matrixDir))
                 {
                      neighborAtLabel(matrixDir).putToken(takeToken<MatrixToken>());
                }
                else
                {
                    matrixFlag = matrixDir;
                 }
            }
            else//recruit or pass to new vector
            {
                int vectorDir = (stopReceiveDir+3)%6;
                if(hasNeighborAtLabel(vectorDir) && neighborAtLabel(vectorDir).state == State::Vector
                        && noTokensAtLabel(vectorDir))
                {
                    neighborAtLabel(vectorDir).putToken(takeToken<MatrixToken>());
                }
                else
                {
                    vectorFlag = vectorDir;
                }

            }
        }


    }
    else if(state == State::Matrix) {
        if(!setlocValue && countTokens<MatrixToken>()>0){
            std::shared_ptr<MatrixToken> mvToken = takeToken<MatrixToken>();
            locationValue = mvToken->value;
            setlocValue = true;
        }
        if(setlocValue && countTokens<MatrixToken>()>0)
        {

                int matrixDir = (stopReceiveDir+3)%6;
                followDir = matrixDir;
                if(hasNeighborAtLabel(matrixDir) && neighborAtLabel(matrixDir).state == State::Matrix
                        && neighborAtLabel(matrixDir).countTokens<MatrixToken>()==0)
                {
                    neighborAtLabel(matrixDir).putToken(takeToken<MatrixToken>());
                }
                else
                {
                        matrixFlag = matrixDir;
                }

        }

        if(setlocValue && countTokens<VectorToken>()>0)//require follow dir here so handling tokens done in 1 step, no waiting
        {
            //do multiplication
            std::shared_ptr<VectorToken> vtoken = takeToken<VectorToken>();
            int product = locationValue*vtoken->value;
            for(int i =0; i<product; i++)
            {
                putToken(std::make_shared<SumToken>());
            }
            sentProduct = true;

            //send forward if possible
            int matrixDir = (stopReceiveDir+3)%6;
            if(hasNeighborAtLabel(matrixDir))
            {
                qDebug()<<"Pass vtoken";
                neighborAtLabel(matrixDir).putToken(vtoken);

            }

        }

        //pass as much as possible
      if(setlocValue)
      {
          int matrixDir = (stopReceiveDir+3)%6;
          int acrossDir = (matrixDir+2)%6;
          if(countTokens<SumToken>()>0)
          {
                if(hasNeighborAtLabel(acrossDir) && (neighborAtLabel(acrossDir).state== State::Matrix || neighborAtLabel(acrossDir).state==State::Result))
                {
                    /* while(neighborAtLabel(acrossDir).countTokens<SumToken>()<tokenMax && countTokens<SumToken>()>0){
                        neighborAtLabel(acrossDir).putToken(takeToken<SumToken>());
                     }*/
                }
                else
                {
                    resultFlag = acrossDir;
                }
          }
        }
        /*if(followDir!=-1&& !hasNeighborAtLabel((followDir+1)%6) && !hasNeighborAtLabel((followDir+2)%6)
                && countTokens<SumToken>()==0 && sentProduct){
            state = State::Finish;
        }
        else if(followDir!=-1 && hasNeighborAtLabel((followDir+2)%6)  && neighborAtLabel((followDir+2)%6).state==State::Finish
                && countTokens<SumToken>()==0){
            state=State::Finish;
        }*/
    }

    else if (state == State::Result){
        followDir = stopReceiveDir;
        /*if(followDir<0)
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

        if(followDir!=-1 && hasNeighborAtLabel((followDir+3)%6)  && neighborAtLabel((followDir+3)%6).state==State::Finish
                && countTokens<SumToken>()<tokenMax){
                   state=State::Finish;
               }*/
    }
    else{
        state=State::Finish;
    }

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
        else if (setlocValue)
            return 0x006600;
        return 0x00ff00;
    case State::Result:
        if (countTokens<SumToken>()>0)
                    return 0xff0000;
        return 0x0000ff;
    case State::Vector:
        if(setlocValue)
            return 0x221448;
         return  0x551A8B;
    }

    return -1;
}
bool Matrix2Particle::noTokensAtLabel(int label)
{
    return (neighborAtLabel(label).countTokens<MatrixToken>() ==0 && neighborAtLabel(label).countTokens<VectorToken>() ==0&&
            neighborAtLabel(label).countTokens<EndOfColumnToken>() ==0 && neighborAtLabel(label).countTokens<EndOfVectorToken>() == 0 &&
            neighborAtLabel(label).countTokens<StartMultToken>()==0);
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
int Matrix2Particle::tryVectorStop() const
{
    auto propertyCheck = [&](const Matrix2Particle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Matrix || p.state == State::Vector || p.state==State::Result) && p.vectorFlag>=0 &&
                pointsAtMe(p, p.vectorFlag);
    };

   return labelOfFirstNeighborWithProperty<Matrix2Particle>(propertyCheck);
}
int Matrix2Particle::tryMatrixStop() const
{

    auto propertyCheck = [&](const Matrix2Particle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Matrix || p.state == State::Vector || p.state==State::Result) && p.matrixFlag>=0 &&
                pointsAtMe(p, p.matrixFlag);
    };

    return labelOfFirstNeighborWithProperty<Matrix2Particle>(propertyCheck);
}
int Matrix2Particle::tryResultStop() const
{

    auto propertyCheck = [&](const Matrix2Particle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Matrix || p.state == State::Vector || p.state==State::Result) && p.resultFlag>=0 &&
                pointsAtMe(p, p.resultFlag);
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
    moveDir = labelOfFirstNeighborInState({State::Seed, State::Finish,State::Matrix,State::Vector,State::Result});
    while(hasNeighborAtLabel(moveDir) && (neighborAtLabel(moveDir).state == State::Seed || neighborAtLabel(moveDir).state == State::Finish
                                          || neighborAtLabel(moveDir).state == State::Matrix
                                          || neighborAtLabel(moveDir).state == State::Vector
                                          || neighborAtLabel(moveDir).state == State::Result)) {
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
    double holeProb = 0.0;
    // numParticles = 11;

      insert(new Matrix2Particle(Node(0, 0), -1, randDir(), *this, Matrix2Particle::State::Seed));


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
            insert(new Matrix2Particle(randomCandidate, -1, randDir(), *this, Matrix2Particle::State::Idle));
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
  /* Setup for pre-configured matrix
   *
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
*/

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
