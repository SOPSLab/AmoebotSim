#include <set>

#include <QtGlobal>

#include "alg/EdgeDetect.h"

#include <QDebug>
EdgeDetectParticle::EdgeDetectParticle(const Node head,
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
      stopFlag(-1),
      stopReceiveDir(-1),
      columnFinished(false),
      lastCol(false),
      resultRound(0),
      vectorLeftover(-1),
      pixelVal(-1),
      gradientMagnitude(-1),
      gradientDir(-10)
{
   for(int i =0;i<8;i++)
   {
       gridvals[i]=-1;
   }
}
void EdgeDetectParticle::setCounterGoal(int goal){

}
void EdgeDetectParticle::activate()
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
            if(countTokens<StreamToken>() ==0 &&
                 (valueStream.size()==0 || streamIter<valueStream.size() ))
            {
                if(streamIter==0)
                {
                    std::ifstream myfile("/Users/Alex/amoebotsim/alg/matrixstream_image.txt");
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
                    std::shared_ptr<StreamToken> eocToken= std::make_shared<StreamToken>();
                    eocToken->type = TokenType::EndOfColumnToken;
                    putToken(eocToken);

                }
                else if(valueStream[streamIter].compare("eov")==0)
                {
                    sMode = 2;
                    qDebug()<<"end of vector";
                    std::shared_ptr<StartMultToken> startmult= std::make_shared<StartMultToken>();
                    putToken(startmult);
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
                        std::shared_ptr<StreamToken> mvToken= std::make_shared<StreamToken>();
                        mvToken->type = TokenType::MatrixToken;
                        mvToken->value = newValue;
                        putToken(mvToken);
                        vectorFlag = 0;

                    }
                    else if (sMode ==1)//vector
                    {
                        std::shared_ptr<StreamToken> vvToken= std::make_shared<StreamToken>();
                        vvToken->type = TokenType::VectorToken;
                        vvToken->value = newValue;
                        putToken(vvToken);

                    }

                }
                streamIter++;


            }
            else if (countTokens<StreamToken>()>0)
            {
                 std::shared_ptr<StreamToken> stoken = peekAtToken<StreamToken>();//TODO: replace with peek so no put backs
                 if(stoken->type == TokenType::MatrixToken)
                 {
                     //continue trying to recruit or if had to wait for space to open up
                     if(hasNeighborAtLabel(0) && neighborAtLabel(0).state == State::Vector && noTokensAtLabel(0))
                     {

                         neighborAtLabel(0).putToken(takeToken<StreamToken>());
                         qDebug()<<"put neighbor matrix token";
                     }

                 }
                 else if (stoken->type == TokenType::VectorToken)
                 {
                     if(hasNeighborAtLabel(0) && neighborAtLabel(0).state == State::Vector && noTokensAtLabel(0))
                     {

                         neighborAtLabel(0).putToken(takeToken<StreamToken>());
                         qDebug()<<"put neighbor vector token";
                     }

                 }
                 else if(stoken->type ==TokenType::EndOfColumnToken && noTokensAtLabel(0))
                 {
                     neighborAtLabel(0).putToken(takeToken<StreamToken>());
                     qDebug()<<"put neighbor eoc token";
                 }

            }
            if(countTokens<StartMultToken>()>0)
            {
                if(hasNeighborAtLabel(0) && neighborAtLabel(0).state == State::Vector && noTokensAtLabel(0))
                {
                    neighborAtLabel(0).putToken(takeToken<StartMultToken>());
                }
            }

            return;
        }
        else if(state == State::Idle) {
            if(hasNeighborInState({State::Seed, State::Matrix,State::Vector,State::Result,State::Prestop})) {
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
            if(hasNeighborInState({State::Seed, State::Matrix,State::Vector,State::Result,State::Prestop})) {
                state = State::Lead;
                updateMoveDir();
                return;
            } else if(hasTailAtLabel(followDir)) {
                EdgeDetectParticle neighbor = neighborAtLabel(followDir);
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
                stopFlag = (stopReceiveDir+3)%6;
                //  return;
            }
            else if (matrixStop!=-1){
                state = State::Matrix;
                stopReceiveDir = matrixStop;
                int matrixDir = (stopReceiveDir+3)%6;
                followDir = matrixDir;
                // return;
            }
            else if (resultStop!=-1){
                state = State::Result;
                stopReceiveDir = resultStop;
                //   return;
            }
            else if (shouldStop())
            {

                state = State::Prestop;
                qDebug()<<"Is prestop";
            }
            else if (tunnelCheck())
            {
                for(int i =0; i<6;i++)
                {
                    if(!hasNeighborAtLabel(i))
                    {
                        qDebug()<<"tunnel move dir";
                        moveDir = i;
                        if(!hasNeighborAtLabel(moveDir)) {
                            expand(moveDir);
                        } else if(hasTailAtLabel(moveDir)) {
                            push(moveDir);
                        }
                    }
                }
            }
            else if(!stopFlagReceived()) {
                updateMoveDir();
                if(!hasNeighborAtLabel(moveDir)) {
                    expand(moveDir);
                } else if(hasTailAtLabel(moveDir)) {
                    push(moveDir);
                }
                //return;
            }
        }
        else if (state == State::Prestop)
        {
            int vectorStop = tryVectorStop();
            int matrixStop = tryMatrixStop();
            int resultStop = tryResultStop();
            if(vectorStop !=-1) {
                qDebug()<<"vector stop";
                state = State::Vector;
                stopReceiveDir = vectorStop;
                stopFlag = (stopReceiveDir+3)%6;
                //  return;
            }
            else if (matrixStop!=-1){
                state = State::Matrix;
                stopReceiveDir = matrixStop;
                int matrixDir = (stopReceiveDir+3)%6;
                followDir = matrixDir;
                // return;
            }
            else if (resultStop!=-1){
                state = State::Result;
                stopReceiveDir = resultStop;
                //   return;
            }
        }
        else if(state == State::Vector) {
            if(countTokens<StreamToken>()>0)
            {
                //take first stream token (FIFO)- can only really act on one, holding two is just for efficiency
                std::shared_ptr<StreamToken> stoken = peekAtToken<StreamToken>();
                if(stoken->type == TokenType::VectorToken)
                {
                    if(!setlocValue)
                    {
                        locationValue = stoken->value;
                        setlocValue = true;
                        //discard (not needed to do multiplication)
                        takeToken<StreamToken>();
                    }
                    else
                    {
                        int vectorDir = (stopReceiveDir+3)%6;
                        if(noTokensAtLabel(vectorDir))
                        {
                            neighborAtLabel(vectorDir).putToken(takeToken<StreamToken>());
                        }

                    }
                }
                else if (stoken->type == TokenType::EndOfColumnToken)
                {
                    if(!columnFinished)
                    {
                        columnFinished = true;
                        qDebug()<<"column finished";
                        //let token die
                        std::shared_ptr<StreamToken> stoken= takeToken<StreamToken>();
                        stoken = NULL;
                    }
                    else
                    {
                        int vectorDir = (stopReceiveDir+3)%6;
                        if(noTokensAtLabel(vectorDir))
                        {
                            neighborAtLabel(vectorDir).putToken(takeToken<StreamToken>());
                        }

                    }
                }
                else if (stoken->type ==TokenType::MatrixToken)
                {
                    if(!columnFinished)
                    {
                        int matrixDir = (stopReceiveDir+1)%6;
                        followDir = matrixDir;
                        if(hasNeighborAtLabel(matrixDir) && neighborAtLabel(matrixDir).state == State::Matrix
                                && noTokensAtLabel(matrixDir))
                        {
                            neighborAtLabel(matrixDir).putToken(takeToken<StreamToken>());
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
                            neighborAtLabel(vectorDir).putToken(takeToken<StreamToken>());
                        }
                        else
                        {
                            vectorFlag = vectorDir;
                        }

                    }
                }
            }

            if (countTokens<StartMultToken>()>0)
            {
                int vectorDir = (stopReceiveDir+3)%6;
                int matrixDir = (stopReceiveDir+1)%6;
                if (vectorDir>=0 && hasNeighborAtLabel(vectorDir) && neighborAtLabel(vectorDir).state == State::Vector )
                {
                    if(  neighborAtLabel(vectorDir).countTokens<StreamToken>()==0)
                    {
                        neighborAtLabel(vectorDir).putToken(takeToken<StartMultToken>());
                    }
                }
                else
                {
                    neighborAtLabel(matrixDir).putToken(takeToken<StartMultToken>());
                }
            }


        }
        else if(state == State::Matrix) {
            //take first stream token (FIFO)- can only really act on one, holding two is just for efficiency
            if(countTokens<StreamToken>()>0)
            {
                std::shared_ptr<StreamToken> stoken = peekAtToken<StreamToken>();
                if(!setlocValue && stoken->type==TokenType::MatrixToken){
                    std::shared_ptr<StreamToken> mvToken = takeToken<StreamToken>();
                    locationValue = mvToken->value;
                    pixelVal = locationValue;
                    setlocValue = true;
                }
                else if(setlocValue && stoken->type==TokenType::MatrixToken)
                {

                    int matrixDir = (stopReceiveDir+3)%6;
                    followDir = matrixDir;
                    if(hasNeighborAtLabel(matrixDir) && neighborAtLabel(matrixDir).state == State::Matrix
                            && noTokensAtLabel(matrixDir))
                    {
                        neighborAtLabel(matrixDir).putToken(takeToken<StreamToken>());
                    }
                    else
                    {
                        matrixFlag = matrixDir;
                    }
                }
            }
            if(completeNeighborhood())
            {
                //get neighbors first time
                    if(gridvals[1]==-1)
                    {
                       gridvals[5] = neighborAtLabel(followDir).pixelVal;
                        gridvals[6] = neighborAtLabel((followDir+1)%6).pixelVal;
                      gridvals[7] = neighborAtLabel((followDir+2)%6).pixelVal;
                      gridvals[1] = (int)neighborAtLabel((followDir+3)%6).pixelVal;
                      gridvals[2] = (int)neighborAtLabel((followDir+4)%6).pixelVal;
                      gridvals[3] = (int)neighborAtLabel((followDir+5)%6).pixelVal;
                }

                //subsequently try to acquire 2-hop
                if(gridvals[0]==-1)
                {
                       gridvals[0] = getTwoHopVal((followDir+3)%6,2);
                }
                if(gridvals[4]==-1)
                {

                    gridvals[4] = getTwoHopVal(followDir,2);

                }
                //all good->do calc
                if(!missingGridVal())
                {
                    qDebug()<<"has all: "<<pixelVal<<" 0-7: "<<gridvals[0]<<gridvals[1]<<gridvals[2]<<gridvals[3]<<gridvals[4]<<gridvals[5]<<gridvals[6]<<gridvals[7];
                  double  Gx = gridvals[6]*-1+gridvals[4]+gridvals[7]*-2+gridvals[3]*2+gridvals[2]+gridvals[0]*-1;
                   double  Gy = gridvals[6]+gridvals[5]*2+gridvals[4]+gridvals[0]*-1+gridvals[1]*-2+gridvals[2]*-1;
                    gradientMagnitude = sqrt(Gx*Gx+Gy*Gy);
                    gradientDir = atan(Gx/Gy);
                    qDebug()<<"magnitude: "<<gradientMagnitude<<" angle: "<<gradientDir;
                }
            }

        }

        else if (state == State::Result){
            followDir = stopReceiveDir;
            int acrossDir = (stopReceiveDir+3)%6;
            if(countTokens<SumToken>()==tokenMax )
            {
                if(hasNeighborAtLabel(acrossDir) && neighborAtLabel(acrossDir).state==State::Result)
                {

                    if(  neighborAtLabel(acrossDir).countTokens<SumToken>()<tokenMax )
                    {
                      //  qDebug()<<"results carryover: "<<countTokens<SumToken>();
                        //pass 1 (carryover)
                        neighborAtLabel(acrossDir).putToken(takeToken<SumToken>());
                        //discard the rest
                        int discardcount = 0;
                        while(countTokens<SumToken>()>0){
                            discardcount ++;
                            takeToken<SumToken>();
                        }
                      //  qDebug()<<"discarded: "<<discardcount;
                    }

                }
                else
                {
                   // qDebug()<<"result flagged";
                    resultFlag = acrossDir;
                }
            }

        }
        else{
            state=State::Finish;
        }

    }


}





int EdgeDetectParticle::headMarkColor() const
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
        if(gradientMagnitude>-1)
        {
            int r = (int)((double)gradientMagnitude/500.0*255);
           int g =0;
           int b=  0;
            return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
        }
        if(setlocValue)
        {
            int colorvalue = locationValue*1;
          //  if (colorvalue>255)colorvalue =255;
            return colorvalue;
        }
       /* if (setlocValue && locationValue>200)
             return 0x0000ff;
        else
            return 0xffffff;*/
    case State::Result:
        if (countTokens<SumToken>()==1)
              return 0xD6C19B;//lightest brown
        if (countTokens<SumToken>()==2)
            return 0x9a6605;//light brown
        if (countTokens<SumToken>()==3)
            return 0x2b1d0e;//dark brown

        return 0xFFA500;//orange-ish
    case State::Vector:
        if(setlocValue)
            return 0x221448;
        return  0x551A8B;
    case State::Prestop:
          return 0xAD0101;

    }

    return -1;
}
bool EdgeDetectParticle::noTokensAtLabel(int label)
{
    return neighborAtLabel(label).countTokens<StreamToken>()<1;
        //(neighborAtLabel(label).countTokens<MatrixToken>() ==0 && neighborAtLabel(label).countTokens<VectorToken>() ==0&&
          //  neighborAtLabel(label).countTokens<EndOfColumnToken>() ==0 && neighborAtLabel(label).countTokens<EndOfVectorToken>() == 0
          //  &&neighborAtLabel(label).countTokens<StartMultToken>()==0);
}

int EdgeDetectParticle::headMarkDir() const
{
    if(state == State::Lead) {
        return moveDir;
    } else if(state == State::Seed || state == State::Finish) {
        return constructionDir;
    } else if(state == State::Follow || state== State::Active || state ==State::Matrix || state == State::Vector || state == State::Result) {
        if(gradientDir>-10)
        {
            double twopi =  2*3.1415926535;
            double posdir = gradientDir+twopi;
            double ratio = posdir/twopi;
            int offset = (int)(ratio*6.0);
            return (followDir+offset)%6;
        }
        return followDir;
    }
    return -1;
}

int EdgeDetectParticle::tailMarkColor() const
{
    return headMarkColor();
}

QString EdgeDetectParticle::inspectionText() const
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
    text += "\n Stream Token: " + QString::number(countTokens<StreamToken>());
    text += "\nDisplay: "+QString::number(displayVal)+"\n";
    return text;
}

EdgeDetectParticle& EdgeDetectParticle::neighborAtLabel(int label) const
{
    return AmoebotParticle::neighborAtLabel<EdgeDetectParticle>(label);
}

int EdgeDetectParticle::labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel) const
{
    auto propertyCheck = [&](const EdgeDetectParticle& p) {
        for(auto state : states) {
            if(p.state == state) {
                return true;
            }
        }
        return false;
    };

    return labelOfFirstNeighborWithProperty<EdgeDetectParticle>(propertyCheck, startLabel);
}

bool EdgeDetectParticle::hasNeighborInState(std::initializer_list<State> states) const
{
    return labelOfFirstNeighborInState(states) != -1;
}

int EdgeDetectParticle::constructionReceiveDir() const
{
    auto propertyCheck = [&](const EdgeDetectParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Finish) &&
                pointsAtMe(p, p.constructionDir);
    };

    return labelOfFirstNeighborWithProperty<EdgeDetectParticle>(propertyCheck);
}
bool EdgeDetectParticle::stopFlagReceived() const
{
    auto propertyCheck = [&](const EdgeDetectParticle& p) {
        return  isContracted() &&
                (p.state == State::Vector) &&
                pointsAtMe(p, p.stopFlag);
    };
    if ( labelOfFirstNeighborWithProperty<EdgeDetectParticle>(propertyCheck)!=-1)
    {
        return true;
    }
     return false;

}

int EdgeDetectParticle::tryVectorStop() const
{
    auto propertyCheck = [&](const EdgeDetectParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Matrix || p.state == State::Vector || p.state==State::Result) && p.vectorFlag>=0 &&
                pointsAtMe(p, p.vectorFlag);
    };

    return labelOfFirstNeighborWithProperty<EdgeDetectParticle>(propertyCheck);
}
int EdgeDetectParticle::tryMatrixStop() const
{

    auto propertyCheck = [&](const EdgeDetectParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Matrix || p.state == State::Vector || p.state==State::Result) && p.matrixFlag>=0 &&
                pointsAtMe(p, p.matrixFlag);
    };

    return labelOfFirstNeighborWithProperty<EdgeDetectParticle>(propertyCheck);
}
int EdgeDetectParticle::tryResultStop() const
{

    auto propertyCheck = [&](const EdgeDetectParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Matrix || p.state == State::Vector || p.state==State::Result) && p.resultFlag>=0 &&
                pointsAtMe(p, p.resultFlag);
    };

    return labelOfFirstNeighborWithProperty<EdgeDetectParticle>(propertyCheck);
}
bool EdgeDetectParticle::completeNeighborhood() const
{
    int count = 0;
    for(int i =0; i<6; i++)
    {
        if(hasNeighborAtLabel(i) && neighborAtLabel(i).state==State::Matrix && neighborAtLabel(i).pixelVal>-1)
        {
            count++;
        }
    }
    return (count == 6);
}
bool EdgeDetectParticle::missingGridVal() const
{
        for (int i =0; i<8;i++)
        {
            if(gridvals[i]==-1)
            {
                return true;
            }
        }
        return false;
}
int EdgeDetectParticle::getTwoHopVal(int sourceNeighbor,int offset) const
{
    for(int i =0; i<6;i++)
    {
        if(pointsAtMe(neighborAtLabel(sourceNeighbor),i))
         {
                int myID = i;
                int targetDir = (myID+offset)%6;
                return neighborAtLabel(sourceNeighbor).neighborAtLabel(targetDir).pixelVal;//gridvals[targetDir];
        }

    }
    return -1;
}



bool EdgeDetectParticle::canFinish() const
{
    return constructionReceiveDir() != -1;
}

void EdgeDetectParticle::updateConstructionDir()
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

void EdgeDetectParticle::updateMoveDir()
{
    moveDir = labelOfFirstNeighborInState({State::Seed, State::Finish,State::Matrix,State::Vector,State::Result,State::Prestop});
    while(hasNeighborAtLabel(moveDir) && (neighborAtLabel(moveDir).state == State::Seed || neighborAtLabel(moveDir).state == State::Finish
                                          || neighborAtLabel(moveDir).state == State::Matrix
                                          || neighborAtLabel(moveDir).state == State::Vector
                                          || neighborAtLabel(moveDir).state == State::Result
                                          || neighborAtLabel(moveDir).state == State::Prestop
                                         )) {
        moveDir = (moveDir + 5) % 6;
    }
}

bool EdgeDetectParticle::hasTailFollower() const
{
    auto propertyCheck = [&](const EdgeDetectParticle& p) {
        return  p.state == State::Follow &&
                pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
    };
    return labelOfFirstNeighborWithProperty<EdgeDetectParticle>(propertyCheck) != -1;
}
bool EdgeDetectParticle::tunnelCheck() const
{
    int firstLabel = labelOfFirstNeighborInState({State::Lead});
    if(firstLabel!=-1 && hasNeighborAtLabel((firstLabel+1)%6) && neighborAtLabel((firstLabel+1)%6).state==State::Lead &&
            hasNeighborAtLabel((firstLabel+2)%6) && neighborAtLabel((firstLabel+2)%6).state==State::Result &&
            hasNeighborAtLabel((firstLabel+5)%6) && neighborAtLabel((firstLabel+5)%6).state==State::Result)
    {
        qDebug()<<"fix tunnel";
        return true;
    }
    if(firstLabel!=-1 && hasNeighborAtLabel((firstLabel+5)%6) && neighborAtLabel((firstLabel+5)%6).state==State::Lead &&
            hasNeighborAtLabel((firstLabel+1)%6) && neighborAtLabel((firstLabel+1)%6).state==State::Result &&
            hasNeighborAtLabel((firstLabel+4)%6) && neighborAtLabel((firstLabel+4)%6).state==State::Result)
    {
        qDebug()<<"fix tunnel";

        return true;
    }
    return false;
}
bool EdgeDetectParticle::shouldStop() const
{
    int firstLabel = labelOfFirstNeighborInState({State::Vector});

    if(firstLabel!=-1 && hasNeighborAtLabel((firstLabel+1)%6) && neighborAtLabel((firstLabel+1)%6).state==State::Vector && hasNeighborInState({State::Matrix}))
    {
        return true;

    }
    if(firstLabel!=-1 && hasNeighborAtLabel((firstLabel+5)%6) && neighborAtLabel((firstLabel+5)%6).state==State::Vector && hasNeighborInState({State::Matrix}))
    {
        return true;

    }
     firstLabel = labelOfFirstNeighborInState({State::Matrix});

    if(hasNeighborInState({State::Prestop}) && firstLabel!=-1 && hasNeighborAtLabel((firstLabel+1)%6) && neighborAtLabel((firstLabel+1)%6).state==State::Matrix && hasNeighborInState({State::Matrix}))
    {
        return true;

    }
    if(hasNeighborInState({State::Prestop}) && firstLabel!=-1 && hasNeighborAtLabel((firstLabel+5)%6) && neighborAtLabel((firstLabel+5)%6).state==State::Matrix && hasNeighborInState({State::Matrix}))
    {
        return true;

    }

     if( firstLabel!=-1 && hasNeighborAtLabel((firstLabel+1)%6) && neighborAtLabel((firstLabel+1)%6).state==State::Matrix
            &&hasNeighborAtLabel((firstLabel+2)%6) && neighborAtLabel((firstLabel+2)%6).state==State::Matrix )
    {
        return true;
    }
     if( firstLabel!=-1 && hasNeighborAtLabel((firstLabel+5)%6) && neighborAtLabel((firstLabel+5)%6).state==State::Matrix
            &&hasNeighborAtLabel((firstLabel+4)%6) && neighborAtLabel((firstLabel+4)%6).state==State::Matrix )
    {
        return true;
    }
     if( firstLabel!=-1 && hasNeighborAtLabel((firstLabel+5)%6) && neighborAtLabel((firstLabel+5)%6).state==State::Matrix
            &&hasNeighborAtLabel((firstLabel+1)%6) && neighborAtLabel((firstLabel+1)%6).state==State::Matrix )
    {
        return true;
    }
     firstLabel = labelOfFirstNeighborInState({State::Prestop});
     if( firstLabel!=-1 && hasNeighborAtLabel((firstLabel+5)%6) && neighborAtLabel((firstLabel+5)%6).state==State::Prestop
            &&hasNeighborAtLabel((firstLabel+1)%6) && neighborAtLabel((firstLabel+1)%6).state==State::Prestop )
    {
        return true;
    }
     if( firstLabel!=-1 && hasNeighborAtLabel((firstLabel+2)%6) && neighborAtLabel((firstLabel+2)%6).state==State::Prestop)
     {
        return true;
    }
     if( firstLabel!=-1 && hasNeighborAtLabel((firstLabel+4)%6) && neighborAtLabel((firstLabel+4)%6).state==State::Prestop)
     {
        return true;
    }

   //     return true;
   // if(hasNeighborInState({State::Vector}) && hasNeighborInState({State::Matrix}))
   //     return true;

    return false;
}

EdgeDetectSystem::EdgeDetectSystem(int numParticles, int countValue)
{
    double holeProb = 0.0;
    // numParticles = 11;

    insert(new EdgeDetectParticle(Node(0, 0), -1, randDir(), *this, EdgeDetectParticle::State::Seed));


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
            insert(new EdgeDetectParticle(randomCandidate, -1, randDir(), *this, EdgeDetectParticle::State::Idle));
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

bool EdgeDetectSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    if(!isConnected(particles)) {
        return true;
    }
#endif

    for(auto p : particles) {
        auto hp = dynamic_cast<EdgeDetectParticle*>(p);
        if(hp->state != EdgeDetectParticle::State::Seed && hp->state != EdgeDetectParticle::State::Finish) {
            return false;
        }
    }

    return true;
}
