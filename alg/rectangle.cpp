#include <set>

#include <QtGlobal>

#include "alg/rectangle.h"
#include <QDebug>

RectangleParticle::RectangleParticle(const Node head,
                                     const int globalTailDir,
                                     const int orientation,
                                     AmoebotSystem& system,
                                     State state)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      state(state),
      constructionDir(-1),
      moveDir(-1),
      followDir(-1),
      buildDir(-1),
      fillDir1(-1),
      fillDir2(-1),
      hasSentPC(false),
      hasConsumedPC(false),
      buildDirReverse(-1)
{
    if(state == State::Seed) {
        constructionDir = 0;
    }
}

void RectangleParticle::activate()
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
            if (buildDir ==-1){
                std::shared_ptr<SToken> stoken= std::make_shared<SToken>();
                stoken->value= height;
                putToken(stoken);
                std::shared_ptr<PCPrepToken> pcprep= std::make_shared<PCPrepToken>();
                pcprep->ontoken = true;
                putToken(pcprep);

                buildDir = 1;

            }

            if(buildDir ==1 && hasNbrAtLabel(buildDir) && nbrAtLabel(buildDir).state==State::Finish &&
                    countTokens<PCPrepToken>()>0 &&  nbrAtLabel(buildDir).countPCwithValue(0)==0 ){
                std::shared_ptr<PCPrepToken> pcprep = takeToken<PCPrepToken>();
                //first row- will be on automatically
                nbrAtLabel(buildDir).putToken(makePCWithValue(width-1));//start with 0 because self took PC1



            }
            //Row corresponding to seed has been finished
            if(countTokens<RoundEnd2Token>()>0 ){
                buildDir = 0;
            }
            if(buildDir == 0 && hasNbrAtLabel(buildDir) && nbrAtLabel(buildDir).state == State::Finish){
                if(countTokens<SToken>()>0)
                {
                    std::shared_ptr<SToken> stoken = takeToken<SToken>();
                    int oldvalue = stoken->value;
                    stoken->value = oldvalue -1;
                    if(stoken->value <0)
                        stoken->value = height;
                    nbrAtLabel(buildDir).putToken(stoken);
                }
                if(countTokens<PCPrepToken>()>0)
                {
                    if( nbrAtLabel(1).countPCwithValue(width)==0 ){
                        std::shared_ptr<PCPrepToken> pcprep = takeToken<PCPrepToken>();
                        if(pcprep->ontoken){
                            qDebug()<<"PC prep is on";
                            nbrAtLabel(1).putToken(makePCWithValue(width)); //from further down so start as PC1's
                        }
                        else
                        {
                            nbrAtLabel(1).putToken(makePCWithValue(-1));
                        }

                    }
                }
            }
            return;
        } else if(state == State::Idle) {
            if(hasNeighborInState({State::Seed, State::Finish})) {
                state = State::Lead;
                updateMoveDir();
                return;
            } else if(hasNeighborInState({State::Lead, State::Follow})) {
                state = State::Follow;
                followDir = labelOfFirstNeighborInState({State::Lead, State::Follow});
                return;
            }
        } else if(state == State::Follow) {
            if(hasNeighborInState({State::Seed, State::Finish})) {
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
            if(checkFinish()==1) {
                state = State::Finish;
                // updateConstructionDir();
                return;
            }
            else if (checkFinish()==2)
            {
                state = State::Finish;
            }
            else {
                updateMoveDir();
                if(!hasNbrAtLabel(moveDir)) {
                    expand(moveDir);
                } else if(hasTailAtLabel(moveDir)) {
                    push(moveDir);
                }
            }
            return;
        }
        else if (state == State::Finish)
        {

            if(countTokens<SToken>()>0 && !hasSentPC && hasNbrAtLabel(buildDirReverse))
            {
                std::shared_ptr<SToken> stoken = takeToken<SToken>();

                //look at stoken value, decrement
                int oldvalue = stoken->value;
                stoken->value = oldvalue -1;
                if(stoken->value <0){
                    stoken->value = height;
                }
                putToken(stoken);

                std::shared_ptr<PCPrepToken> pctoken = std::make_shared<PCPrepToken>();
                if(oldvalue ==height)
                {//is off unless at a 0 height
                    pctoken->ontoken = true;
                }
                putToken(pctoken);
                qDebug()<<"finished made PCPrep: "<<pctoken->ontoken;
                nbrAtLabel(buildDirReverse).putToken(takeToken<PCPrepToken>());
                hasSentPC = true;



            }
            if(countTokens<PCPrepToken>()>0 && nbrAtLabel(buildDirReverse).countTokens<PCPrepToken>()==0)
            {
                nbrAtLabel(buildDirReverse).putToken(takeToken<PCPrepToken>());

            }
            //handle countdown tokens as expansion indicators
            if(countPCwithPosValue()>0){
                buildDir = (buildDirReverse+3)%6;
                if(hasNbrAtLabel(buildDir) && nbrAtLabel(buildDir).state==State::Finish
                        && nbrAtLabel(buildDir).countPCwithValue(0)==0)
                {
                    if(!hasConsumedPC)
                    {
                        int currentTokenValue = getPCValue();
                        takeToken<PCToken>() ;
                        nbrAtLabel(buildDir).putToken(makePCWithValue(currentTokenValue-1));
                        hasConsumedPC=true;
                    }
                    else
                    {
                        //transfer token of value 1
                        nbrAtLabel(buildDir).putToken(takeToken<PCToken>());

                    }

                }
            }
            if(countPCwithValue(0)>0){

                if(!hasConsumedPC)
                {
                    takeToken<PCToken>();
                    putToken(std::make_shared<RoundEnd1Token>());
                    hasConsumedPC=true;
                }
                else
                {
                    //move pc0
                    nbrAtLabel(buildDir).putToken(takeToken<PCToken>());

                }

            }
            if (countPCwithValue(-1)>0){
                qDebug()<<"Has PC-1";
                if( buildDir>-1 && hasNbrAtLabel(buildDir) && nbrAtLabel(buildDir).state==State::Finish){
                    nbrAtLabel(buildDir).putToken(takeToken<PCToken>());
                }
                else//must be end of row
                {
                    takeToken<PCToken>();
                    //TODO: problem when height>0
                    putToken(std::make_shared<RoundEnd1Token>());
                }

            }
            //purple wants to go toward 2 received, if it sees it can go down, reset by becoming grey, going out
            //grey wants to go down but if it sees it can go out it should just do so (no reset)
            if(countTokens<RoundEnd1Token>()>0)
            {
                //send forward or backward as needed
                //if has neigbhor at builddir (out)
                if(buildDir>-1 && hasNbrAtLabel(buildDir) && nbrAtLabel(buildDir).state==State::Finish){
                    nbrAtLabel(buildDir).putToken(takeToken<RoundEnd1Token>());
                }
                //also out
                else if(fillDir2>-1 && hasNbrAtLabel(fillDir2) && nbrAtLabel(fillDir2).state==State::Finish){
                    nbrAtLabel(fillDir2).putToken(takeToken<RoundEnd1Token>());
                }

                //else if has neighbor at fill1 ( = down)
                else if(fillDir1>-1 && hasNbrAtLabel(fillDir1) && nbrAtLabel(fillDir1).state==State::Finish){
                    nbrAtLabel(fillDir1).putToken(takeToken<RoundEnd1Token>());
                }

                //else if @ fill2 of neighbor or end of first row, go back that way
                else {
                    bool foundBackDir =false;
                    qDebug()<<"RE1 holder is fill2 end";
                    auto fillPropCheck2 = [&](const RectangleParticle& p) {
                        return   isContracted() &&
                                (p.state == State::Seed || p.state == State::Finish) &&
                                p.fillDir2>=0 &&
                                pointsAtMe(p, (p.fillDir2));
                    };
                    for(int label = 0; label < 6; label++) {
                        if(hasNbrAtLabel(label) && nbrAtLabel(label).state==State::Finish) {
                            RectangleParticle particle = nbrAtLabel(label);
                            if(fillPropCheck2(particle)) {
                                takeToken<RoundEnd1Token>();
                                putToken(std::make_shared<RoundEnd2Token>());
                                foundBackDir = true;
                                qDebug()<<"made RE2";

                            }
                        }
                    }
                    if(!foundBackDir)//first row
                    {
                        qDebug()<<"end of first row.";
                        takeToken<RoundEnd1Token>();
                        nbrAtLabel(buildDirReverse).putToken(std::make_shared<RoundEnd2Token>());
                    }
                }
            }
            if(countTokens<SToken>()>0 )
            {
                if( countTokens<RoundEnd2Token>()>0)
                {
                    qDebug()<<"sholder got RE2";
                    buildDir = (buildDirReverse+3)%6;
                    takeToken<RoundEnd2Token>();
                }
                else if (buildDir>-1 && hasNbrAtLabel(buildDir) && nbrAtLabel(buildDir).state==State::Finish){
                    nbrAtLabel(buildDir).putToken(takeToken<SToken>());
                }

            }
            else if(countTokens<RoundEnd2Token>()>0)
            {
                qDebug()<<"has RE2x"<<countTokens<RoundEnd2Token>();
                if( hasNeighborInState({State::Seed})){
                    int seedLabel= labelOfFirstNeighborInState({State::Seed},0);
                    nbrAtLabel(seedLabel).putToken( takeToken<RoundEnd2Token>());
                }
                else{
                    //If can go down furhter, send a back back up to restart
                    /* if(buildDir>-1 && hasNeighborAtLabel(buildDir) && neighborAtLabel(buildDir).state == State::Finish){
                        takeToken<RoundEnd2Token>();
                       neighborAtLabel(buildDir).putToken(std::make_shared<RoundEnd1Token>());
                       return;
                    }*/
                    if(fillDir1>-1 && hasNbrAtLabel(fillDir1) && nbrAtLabel(fillDir1).state==State::Finish){
                        //  takeToken<RoundEnd2Token>();
                        //  neighborAtLabel(fillDir1).putToken(std::make_shared<RoundEnd1Token>());
                        takeToken<RoundEnd2Token>();
                        putToken(std::make_shared<RoundEnd1Token>());
                        return;
                    }


                    auto fillPropCheck2 = [&](const RectangleParticle& p) {
                        return   isContracted() &&
                                (p.state == State::Seed || p.state == State::Finish) &&
                                p.fillDir2>=0 &&
                                pointsAtMe(p, (p.fillDir2));
                    };
                    auto fillPropCheck1 = [&](const RectangleParticle& p) {
                        return   isContracted() &&
                                (p.state == State::Seed || p.state == State::Finish) &&
                                p.fillDir1>=0 &&
                                pointsAtMe(p, (p.fillDir1));
                    };
                    //if nothing at own fill dirs go backward- only col this wouldn't work on is last, but there is always a RE1
                    if(  (fillDir1<0 || !hasNbrAtLabel(fillDir1) || nbrAtLabel(fillDir1).state!=State::Finish) &&
                         (fillDir2<0 || !hasNbrAtLabel(fillDir2) || nbrAtLabel(fillDir2).state!=State::Finish) &&
                         labelOfFirstNbrWithProperty<RectangleParticle>(fillPropCheck1) ==-1 &&
                         labelOfFirstNbrWithProperty<RectangleParticle>(fillPropCheck2) ==-1){
                        nbrAtLabel(buildDirReverse).putToken(takeToken<RoundEnd2Token>());
                        return;
                    }

                    //normal sending RE2 across
                    for(int label = 0; label < 6; label++) {
                        if(hasNbrAtLabel(label)) {
                            RectangleParticle particle = nbrAtLabel(label);
                            if(fillPropCheck2(particle)) {
                                nbrAtLabel(label).putToken(takeToken<RoundEnd2Token>());
                                return;
                            }

                        }
                    }

                }


                // neighborAtLabel(buildDir).putToken(takeToken<SToken>());
                //   buildDir = (buildDirReverse+3)%6;

                // qDebug()<<"Stoken passed.";
            }
        }
        return;
    }
}

int RectangleParticle::headMarkColor() const
{
    switch(state) {
    case State::Seed:
        return 0x00ff00;
    case State::Idle:   return -1;
    case State::Follow: return 0x0000ff;
    case State::Lead:   return 0xff0000;
    case State::Finish:
        if(countTokens<SToken>()>0)
            return 0xffaa00;//light yello
        else if (countTokens<PCPrepToken>()>0)
            return 0xffff00;//dark yellow
        //else if (countPCwithValue(1)>0)
        //   return 0x0000ff;//blue
        // else if (countPCwithValue(0)>0)
        //     return 0x9999ff;//violet
        else if (countTokens<RoundEnd1Token>()>0)
            return 0x999999;//grey
        else if (countTokens<RoundEnd2Token>()>0)
            return 0x8B008B;//dark purple
        else if (countTokens<RoundEndBToken>()>0)
            return 0xff8888;
        return 0x000000;
        break;
    }

    return -1;
}

int RectangleParticle::headMarkDir() const
{
    if(state == State::Lead) {
        return moveDir;
    } else if(state == State::Seed || state == State::Finish) {
        return constructionDir;
    } else if(state == State::Follow) {
        return followDir;
    }
    return -1;
}

int RectangleParticle::tailMarkColor() const
{
    return headMarkColor();
}

QString RectangleParticle::inspectionText() const
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
        case State::Finish: return "finish";
        }
    }();
    text += "\n";
    return text;
}

RectangleParticle& RectangleParticle::nbrAtLabel(int label) const
{
    return AmoebotParticle::nbrAtLabel<RectangleParticle>(label);
}

int RectangleParticle::labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel) const
{
    auto propertyCheck = [&](const RectangleParticle& p) {
        for(auto state : states) {
            if(p.state == state) {
                return true;
            }
        }
        return false;
    };

    return labelOfFirstNbrWithProperty<RectangleParticle>(propertyCheck, startLabel);
}

bool RectangleParticle::hasNeighborInState(std::initializer_list<State> states) const
{
    return labelOfFirstNeighborInState(states) != -1;
}

int RectangleParticle::constructionReceiveDir() const
{
    auto propertyCheck = [&](const RectangleParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Finish) &&
                pointsAtMe(p, p.constructionDir);
    };

    return labelOfFirstNbrWithProperty<RectangleParticle>(propertyCheck);
}

bool RectangleParticle::canFinish() const
{
    return false;
}
int RectangleParticle::checkFinish()
{
    if(isExpanded())
        return false;
    auto fillPropCheck1 = [&](const RectangleParticle& p) {
        return   isContracted() &&
                (p.state == State::Seed || p.state == State::Finish) &&
                p.fillDir1>=0 &&
                pointsAtMe(p, (p.fillDir1));
    };
    auto fillPropCheck2 = [&](const RectangleParticle& p) {
        return   isContracted() &&
                (p.state == State::Seed || p.state == State::Finish) &&
                p.fillDir2>=0 &&
                pointsAtMe(p, (p.fillDir2));
    };

    auto buildPropCheck = [&](const RectangleParticle& p) {
        return   isContracted() &&
                (p.state == State::Seed || p.state == State::Finish)&&
                p.buildDir>=0 &&
                pointsAtMe(p, (p.buildDir));
    };
    int fillCount = 0;
    int buildCount = 0;
    for(int label = 0; label < 6; label++) {
        if(hasNbrAtLabel(label)) {
            RectangleParticle particle = nbrAtLabel(label);
            if(fillPropCheck1(particle) || fillPropCheck2(particle)) {
                fillCount++;
            }
            if(buildPropCheck(particle)){
                buildCount++;
            }
        }
    }
    //actually check if should be building/filling, for now just propagate flags:
    for(int label = 0; label < 6; label++) {
        if(hasNbrAtLabel(label)) {
            RectangleParticle particle = nbrAtLabel(label);
            if(fillPropCheck1(particle)) {
                fillDir1= (label+3)%6;
            }
            if(fillPropCheck2(particle)) {
                fillDir2= (label+3)%6;
            }

        }
    }
    if (buildCount>0 )
    {
        for(int label = 0; label < 6; label++) {
            if(hasNbrAtLabel(label)) {
                RectangleParticle particle = nbrAtLabel(label);
                if(buildPropCheck(particle)) {
                    fillDir1= (label+2)%6;
                    fillDir2= (label+4)%6;
                    // buildDir = (label+3)%6;
                    buildDirReverse = label;

                }
            }
        }
    }
    return (fillCount>1 || buildCount>0);
}

void RectangleParticle::updateConstructionDir()
{
    constructionDir = constructionReceiveDir();
    if(nbrAtLabel(constructionDir).state == State::Seed) {
        constructionDir = (constructionDir + 1) % 6;
    } else {
        constructionDir = (constructionDir + 2) % 6;
    }

    if(hasNbrAtLabel(constructionDir) && nbrAtLabel(constructionDir).state == State::Finish) {
        constructionDir = (constructionDir + 1) % 6;
    }
}


void RectangleParticle::updateMoveDir()
{
    moveDir = labelOfFirstNeighborInState({State::Seed, State::Finish});
    while(hasNbrAtLabel(moveDir) && (nbrAtLabel(moveDir).state == State::Seed || nbrAtLabel(moveDir).state == State::Finish)) {
        moveDir = (moveDir + 5) % 6;
    }
}
bool RectangleParticle::hasTailFollower() const
{
    auto propertyCheck = [&](const RectangleParticle& p) {
        return  p.state == State::Follow &&
                pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
    };
    return labelOfFirstNbrWithProperty<RectangleParticle>(propertyCheck) != -1;
}
int RectangleParticle::countPCwithValue(int value) {
    //assumes only have 1 PCToken at a time, so that one would/would not have value
    if(countTokens<PCToken>() == 0){
        return 0;
    }
    std::shared_ptr<PCToken> pc  =takeToken<PCToken>();
    int pcvalue = pc->value;
    putToken(pc);
    if (pcvalue == value){
        return 1;
    }
    return 0;
}
int RectangleParticle::countPCwithPosValue() {
    //assumes only have 1 PCToken at a time, so that one would/would not have value
    if(countTokens<PCToken>() == 0){
        return 0;
    }
    std::shared_ptr<PCToken> pc  =takeToken<PCToken>();
    int pcvalue = pc->value;
    putToken(pc);
    if (pcvalue > 0){
        return 1;
    }
    return 0;
}
void RectangleParticle::setPCValue(int value) {
    //assumes only have 1 PCToken at a time, so that one would/would not have value
    Q_ASSERT(countTokens<PCToken>()>0);

    std::shared_ptr<PCToken> pc  =takeToken<PCToken>();
    pc->value = value;
    putToken(pc);

}
int RectangleParticle::getPCValue() {
    if(countTokens<PCToken>() == 0){
        return -1;
    }

    std::shared_ptr<PCToken> pc  =takeToken<PCToken>();
    int pcvalue = pc->value;
    putToken(pc);
    return pcvalue;

}
std::shared_ptr<RectangleParticle::Token> RectangleParticle::makePCWithValue(int value)
{
    std::shared_ptr<PCToken> pc  =std::make_shared<PCToken>();
    pc->value = value;
    return pc;
}


RectangleSystem::RectangleSystem(int numParticles, float holeProb)
{
    insert(new RectangleParticle(Node(0, 0), -1, randDir(), *this, RectangleParticle::State::Seed));

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
            insert(new RectangleParticle(randomCandidate, -1, randDir(), *this, RectangleParticle::State::Idle));
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

bool RectangleSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    if(!isConnected(particles)) {
        return true;
    }
#endif

    for(auto p : particles) {
        auto hp = dynamic_cast<RectangleParticle*>(p);
        if(hp->state != RectangleParticle::State::Seed && hp->state != RectangleParticle::State::Finish) {
            return false;
        }
    }

    return true;
}
