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
      stopFlag(-1),
      stopReceiveDir(-1),
      columnFinished(false),
      lastCol(false),
      resultRound(0),
      vectorLeftover(-1),
      resultEndRow(false)
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
            if(countTokens<StreamToken>() ==0 &&
                    (valueStream.size()==0 || streamIter<valueStream.size() ))
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
                std::shared_ptr<StreamToken> stoken = peekAtToken<StreamToken>();
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
                    state = State::Finish;
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
            if(hasNeighborInState({State::Seed, State::Matrix,State::Vector,State::Result,State::Prestop,State::Finish})) {
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
                if(stopFlagReceived() )//this is for that last row barrier, same as with the vector
                {
                    resultEndRow =true;
                    stopFlag = (stopReceiveDir+3)%6;//in this case we know stop flag and stop receive dir are the same direction
                }
                /* if(hasNeighborInState({State::Vector}))
                {
                   resultFlag = (resultStop+2)%6;
                }
               else if(hasNeighborAtLabel((resultStop+2)%6) && (neighborAtLabel((resultStop+2)%6).state == State::Matrix || neighborAtLabel((resultStop+2)%6).state == State::Result) )
                {
                    resultFlag = (resultStop+3)%6;
                }*/
                //   return;
            }
            else if (shouldStop() )
            {

                state = State::Prestop;
                qDebug()<<"Is prestop";
            }
            /*else if (tunnelCheck())
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
            }*/
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
            /*  else if (resultStop!=-1){
                state = State::Result;
                stopReceiveDir = resultStop;
                if(stopFlagReceived() )//this is for that last row barrier, same as with the vector
                {
                    stopFlag = (stopReceiveDir+3)%6;//in this case we know stop flag and stop receive dir are the same direction
                }
                //   return;
            }*/
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
                        stopFlag = (stopFlag+5)%6;//this is really just for the last one but the rest won't matter
                        //pass down column- trigger multiplication
                        int matrixDir = (stopReceiveDir+1)%6;
                        neighborAtLabel(matrixDir).putToken(takeToken<StreamToken>());
                        std::shared_ptr<ResultCounterToken> rcToken= std::make_shared<ResultCounterToken>();

                        putToken(rcToken);
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
                    qDebug()<<"Vector particle with matrix token";
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
            if(countTokens<ResultCounterToken>()>0)
            {

                int vectorDir = (stopReceiveDir+3)%6;
                if (vectorDir>=0 && hasNeighborAtLabel(vectorDir) && neighborAtLabel(vectorDir).state == State::Vector )
                {
                    while(countTokens<ResultCounterToken>()>0 && neighborAtLabel(vectorDir).countTokens<ResultCounterToken>()<tokenMax)
                    {
                        neighborAtLabel(vectorDir).putToken(takeToken<ResultCounterToken>());
                    }
                }
                else if (setlocValue)
                {
                    resultFlag = stopFlag;//this will be the rotated one
                    if(resultFlag!=-1 && hasNeighborAtLabel(resultFlag) && neighborAtLabel(resultFlag).state==State::Result)
                    {
                        while(countTokens<ResultCounterToken>()>0 && neighborAtLabel(resultFlag).countTokens<ResultCounterToken>()<tokenMax)
                        {
                            neighborAtLabel(resultFlag).putToken(takeToken<ResultCounterToken>());
                        }
                    }
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
                else if(setlocValue && stoken->type==TokenType::VectorToken )//require follow dir here so handling tokens done in 1 step, no waiting
                {
                    //new version: do as much as possible:
                    //ifvectorLeftover ==-1 && has vector token
                    //vectorLeftover = vector value
                    // while 1xown value fits && vectorLeftover>0 (it must be definition, at some point)
                    //make tokens
                    //vectorLeftover--
                    if(vectorLeftover ==-1 )//should be automatic but whatever
                    {
                        int matrixDir = (stopReceiveDir+3)%6;
                        int acrossDir = (matrixDir+2)%6;
                        if(!(hasNeighborAtLabel(matrixDir) && neighborAtLabel(matrixDir).state == State::Matrix) && hasNeighborAtLabel(acrossDir))
                        {
                            qDebug()<<"Last row: "<<(int)(neighborAtLabel(acrossDir).state);
                        }
                        // std::shared_ptr<StreamToken> vtoken = takeToken<StreamToken>();
                        vectorLeftover = stoken->value;
                        sentProduct = true;

                        //send forward if possible
                        if(hasNeighborAtLabel(matrixDir) && neighborAtLabel(matrixDir).state==State::Matrix && neighborAtLabel(matrixDir).setlocValue)
                        {
                            qDebug()<<"Pass vtoken: "<<stoken->value;
                            neighborAtLabel(matrixDir).putToken(takeToken<StreamToken>());

                        }
                        else if(matrixFlag==-1)
                        {
                            takeToken<StreamToken>();
                            qDebug()<<"delete vtoken";
                        }
                        else
                        {
                            vectorLeftover = -1;
                            sentProduct = false;
                        }
                    }



                }

            }
            if(vectorLeftover>0)//this can be executed in same run as when leftover gets set above.
            {
                while(countTokens<SumToken>()+locationValue<=tokenMax && vectorLeftover>0)//while can fit another round
                {
                    for(int i =0; i<locationValue; i++)
                    {
                        putToken(std::make_shared<SumToken>());
                    }
                    vectorLeftover--;
                }
            }

            //pass as much as possible
            if(setlocValue)
            {
                int matrixDir = (stopReceiveDir+3)%6;
                int acrossDir = (matrixDir+2)%6;
                if(!(hasNeighborAtLabel(matrixDir) && neighborAtLabel(matrixDir).state == State::Matrix) && hasNeighborAtLabel(acrossDir))
                {
                    qDebug()<<"Last row: "<<(int)(neighborAtLabel(acrossDir).state)<<"sum tokens: "<<countTokens<SumToken>();
                }
                if(countTokens<SumToken>()>0)
                {
                    if(hasNeighborAtLabel(acrossDir) &&
                            ((neighborAtLabel(acrossDir).state== State::Matrix&&neighborAtLabel(acrossDir).setlocValue)
                             || neighborAtLabel(acrossDir).state==State::Result) &&
                            noTokensAtLabel(acrossDir))
                    {
                        while(neighborAtLabel(acrossDir).countTokens<SumToken>()<tokenMax && countTokens<SumToken>()>0){
                            neighborAtLabel(acrossDir).putToken(takeToken<SumToken>());
                        }
                    }
                    else if(lastCol)
                    {
                        qDebug()<<"result flagged";
                        //resultFlag = acrossDir;
                    }
                }
                if(countTokens<StartMultToken>()>0)
                {
                    lastCol = true;
                    if(hasNeighborAtLabel(matrixDir) && neighborAtLabel(matrixDir).state==State::Matrix )
                    {
                        neighborAtLabel(matrixDir).putToken(takeToken<StartMultToken>());
                    }
                }

            }
            if(countTokens<StreamToken>()==0 && countTokens<SumToken>() ==0 &&
                    vectorLeftover<=0 && neighborAtLabel(stopReceiveDir).state==State::Finish )
            {
                if(!hasNeighborAtLabel((stopReceiveDir+2)%6) || neighborAtLabel((stopReceiveDir+2)%6).state==State::Finish)
                {
                    state = State::Finish;
                }
                if(hasNeighborAtLabel((stopReceiveDir+2)%6)&&(neighborAtLabel((stopReceiveDir+2)%6).state==State::Lead||neighborAtLabel((stopReceiveDir+2)%6).state==State::Follow))
                {
                    state = State::Finish;
                }

            }

        }

        else if (state == State::Result){
            followDir = stopReceiveDir;
            if(stopFlagReceived() )//this is for that last row barrier, same as with the vector
            {
                if(hasNeighborInState({State::Matrix}))
                {
                    stopFlag = (stopReceiveDir+4)%6;
                }
                else
                {
                    stopFlag = (stopReceiveDir+3)%6;//in this case we know stop flag and stop receive dir are the same direction

                }
            }
            int acrossDir = (stopReceiveDir+5)%6;//todo: set properly so that tokens go the right direction
            if(resultEndRow)
            {
                if(hasNeighborInState({State::Matrix,State::Vector}))
                {
                    acrossDir = (stopReceiveDir+4)%6;
                }
                else
                {
                    acrossDir = (stopReceiveDir+3)%6;
                }
            }

            followDir = (acrossDir+3)%6;
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
                    //  resultFlag = acrossDir;
                }
            }
            if(countTokens<ResultCounterToken>()>0)
            {

                //todo: recruit top of next column using actual logarithmic counter
                resultFlag = stopFlag;
                if(resultFlag !=-1 && hasNeighborAtLabel(resultFlag) && neighborAtLabel(resultFlag).state==State::Result)
                {

                    while(countTokens<ResultCounterToken>()>0 && neighborAtLabel(resultFlag).countTokens<ResultCounterToken>()<tokenMax)
                    {
                        neighborAtLabel(resultFlag).putToken(takeToken<ResultCounterToken>());
                    }

                }
            }
            //row is done up to this particle, so will not get any more sum tokens and doesn't need to carry one over
            if(neighborAtLabel(stopReceiveDir).state==State::Finish && countTokens<SumToken>()<tokenMax  && countTokens<ResultCounterToken>()==0 )
            {
                if(resultFlag!=-1 && hasNeighborAtLabel(resultFlag) && neighborAtLabel(resultFlag).state==State::Result)
                {
                    state =State::Finish;
                }
                if(resultFlag==-1)
                {
                    state=State::Finish;
                }
            }


        }
        else{
            state=State::Finish;
        }

    }


}





int Matrix2Particle::headMarkColor() const
{
    if(countTokens<ResultCounterToken>()>0)
        return 0xffffff;
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
        if (numcountsgenerated>1)
            return 0xff0000;
        if(countTokens<StreamToken>()>0)
            return 0xe5ffe5;//light green
        else if (countTokens<SumToken>()==1)
            return 0x99c199;//light green
        else if (countTokens<SumToken>()==2)
            return 0x001900;//even darker green
        else if (countTokens<SumToken>()==3)
            return 0x000000;
        else if (setlocValue && countTokens<SumToken>()==0)
            return 0x006600;//medum green
        qDebug()<<"regular green set loc?"<<(int)setlocValue;
        return 0x00ff00;
        if (setlocValue && locationValue>200)
            return 0x0000ff;
        else
            return 0xffffff;
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
bool Matrix2Particle::noTokensAtLabel(int label)
{
    return neighborAtLabel(label).countTokens<StreamToken>()<1;
    //(neighborAtLabel(label).countTokens<MatrixToken>() ==0 && neighborAtLabel(label).countTokens<VectorToken>() ==0&&
    //  neighborAtLabel(label).countTokens<EndOfColumnToken>() ==0 && neighborAtLabel(label).countTokens<EndOfVectorToken>() == 0
    //  &&neighborAtLabel(label).countTokens<StartMultToken>()==0);
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
    text += "\n Stream Token: " + QString::number(countTokens<StreamToken>());
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
bool Matrix2Particle::stopFlagReceived() const
{
    auto propertyCheck = [&](const Matrix2Particle& p) {
        return  isContracted() &&
                (p.state == State::Vector || p.state==State::Finish ||p.state == State::Result) && p.stopFlag!=-1 &&
                pointsAtMe(p, p.stopFlag);
    };
    if ( labelOfFirstNeighborWithProperty<Matrix2Particle>(propertyCheck)!=-1)
    {
        return true;
    }
    return false;

}
bool Matrix2Particle::hasLastRowNeighbor() const
{
    auto propertyCheck = [&](const Matrix2Particle& p) {
        return  isContracted() &&
                ((p.state == State::Matrix || p.state==State::Finish) &&p.lastCol);
    };
    if ( labelOfFirstNeighborWithProperty<Matrix2Particle>(propertyCheck)!=-1)
    {
        return true;
    }
    return false;

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
    int firstResultFlag = labelOfFirstNeighborWithProperty<Matrix2Particle>(propertyCheck);
    if(firstResultFlag>=0)
    {

        return firstResultFlag;

    }

    int firstresult=  labelOfFirstNeighborInState({State::Result});
    if(firstresult!=-1)
    {
        if(hasNeighborAtLabel((firstresult+1)%6) && hasNeighborAtLabel((firstresult+2)%6) &&
                neighborAtLabel((firstresult+1)%6).state==State::Matrix &&
                neighborAtLabel((firstresult+2)%6).state==State::Matrix )
        {
            return firstresult;
        }
        if(hasNeighborAtLabel((firstresult+1)%6) && hasNeighborAtLabel((firstresult+2)%6) &&
                neighborAtLabel((firstresult+1)%6).state==State::Result &&
                neighborAtLabel((firstresult+2)%6).state==State::Result )
        {
            return firstresult;
        }
        if(hasNeighborAtLabel((firstresult+1)%6) && hasNeighborAtLabel((firstresult+5)%6) &&
                neighborAtLabel((firstresult+1)%6).state==State::Result &&
                neighborAtLabel((firstresult+5)%6).state==State::Result )
        {
            return (firstresult+5)%6;
        }
        if(hasNeighborAtLabel((firstresult+4)%6) && hasNeighborAtLabel((firstresult+5)%6) &&
                neighborAtLabel((firstresult+4)%6).state==State::Result &&
                neighborAtLabel((firstresult+5)%6).state==State::Result )
        {
            return (firstresult+4)%6;
        }
    }

    return -1;
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

bool Matrix2Particle::hasTailFollower() const
{
    auto propertyCheck = [&](const Matrix2Particle& p) {
        return  p.state == State::Follow &&
                pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
    };
    return labelOfFirstNeighborWithProperty<Matrix2Particle>(propertyCheck) != -1;
}
bool Matrix2Particle::tunnelCheck() const
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
bool Matrix2Particle::shouldStop() const
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
    /*if(firstLabel!=-1 && hasNeighborAtLabel((firstLabel+1)%6) && neighborAtLabel((firstLabel+1)%6).state==State::Vector &&
             ( hasNeighborAtLabel((firstLabel+2)%6) && neighborAtLabel((firstLabel+2)%6).state==State::Matrix ||
               hasNeighborAtLabel((firstLabel+2)%6) && neighborAtLabel((firstLabel+2)%6).state==State::Prestop))
*/
    //     return true;
    // if(hasNeighborInState({State::Vector}) && hasNeighborInState({State::Matrix}))
    //     return true;

    return false;
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
