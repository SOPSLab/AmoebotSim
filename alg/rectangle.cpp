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
            //for now just build 1 direction then another, will result in rectangle indefinitely
            if (buildDir ==-1){
                putToken(std::make_shared<SToken>());
                putToken(std::make_shared<PCPrepToken>());
                buildDir = 1;

            }

            if(buildDir ==1 && hasNeighborAtLabel(buildDir) && neighborAtLabel(buildDir).state==State::Finish &&
                    countTokens<PCPrepToken>()>0 &&  neighborAtLabel(buildDir).countTokens<PC0Token>()==0 ){
                takeToken<PCPrepToken>();
                neighborAtLabel(buildDir).putToken(std::make_shared<PC0Token>());//start with 0 because self took PC1

            }
            //Row corresponding to seed has been finished
            if(countTokens<RoundEnd2Token>()>0 ){
                buildDir = 0;
            }
            if(buildDir == 0 && hasNeighborAtLabel(buildDir) && neighborAtLabel(buildDir).state == State::Finish){
                if(countTokens<SToken>()>0)
                    neighborAtLabel(buildDir).putToken(takeToken<SToken>());
                if( countTokens<PCPrepToken>()>0 &&  neighborAtLabel(1).countTokens<PC0Token>()==0 ){
                    takeToken<PCPrepToken>();
                    neighborAtLabel(1).putToken(std::make_shared<PC0Token>());//start with 0 because self took PC1

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
                auto neighbor = neighborAtLabel(followDir);
                int neighborContractionDir = neighborDirToDir(neighbor, (neighbor.tailDir() + 3) % 6);
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
                if(!hasNeighborAtLabel(moveDir)) {
                    expand(moveDir);
                } else if(hasTailAtLabel(moveDir)) {
                    push(moveDir);
                }
            }
            return;
        }
        else if (state == State::Finish)
        {

            if(countTokens<SToken>()>0 && !hasSentPC && hasNeighborAtLabel(buildDirReverse))
            {
                qDebug()<<"finished made PCPrep";
                putToken(std::make_shared<PCPrepToken>());
                neighborAtLabel(buildDirReverse).putToken(takeToken<PCPrepToken>());
                //  buildDir=(buildDirReverse+3)%6;
                hasSentPC = true;

            }
            if(countTokens<PCPrepToken>()>0 && neighborAtLabel(buildDirReverse).countTokens<PCPrepToken>()==0)
            {
                neighborAtLabel(buildDirReverse).putToken(takeToken<PCPrepToken>());

            }
            //handle countdown tokens as expansion indicators
            //TODO: bad ratios happening
            if(countTokens<PC1Token>()>0){
                buildDir = (buildDirReverse+3)%6;
                if(hasNeighborAtLabel(buildDir) && neighborAtLabel(buildDir).state==State::Finish
                        && neighborAtLabel(buildDir).countTokens<PC0Token>()==0)
                {
                    if(!hasConsumedPC)
                    {
                        takeToken<PC1Token>() ;
                        neighborAtLabel(buildDir).putToken(std::make_shared<PC0Token>());
                        hasConsumedPC=true;
                    }
                    else
                    {
                        neighborAtLabel(buildDir).putToken(takeToken<PC1Token>());

                    }

                }
            }
            if(countTokens<PC0Token>()>0){
                buildDir = (buildDirReverse+3)%6;
                if(hasNeighborAtLabel(buildDir) && neighborAtLabel(buildDir).state==State::Finish)
                {
                    if(!hasConsumedPC)
                    {
                        takeToken<PC0Token>() ;
                        putToken(std::make_shared<RoundEnd1Token>());
                        hasConsumedPC=true;
                    }
                    else
                    {
                        neighborAtLabel(buildDir).putToken(takeToken<PC0Token>());

                    }

                }
            }
            if(countTokens<RoundEnd1Token>()>0)
            {
                //send forward or backward as needed
                //if has neigbhor at builddir
                if(buildDir>-1 && hasNeighborAtLabel(buildDir) && neighborAtLabel(buildDir).state==State::Finish){
                    neighborAtLabel(buildDir).putToken(takeToken<RoundEnd1Token>());
                }

                //else if has neighbor at fill1
                else if(fillDir1>-1 && hasNeighborAtLabel(fillDir1) && neighborAtLabel(fillDir1).state==State::Finish){
                    neighborAtLabel(fillDir1).putToken(takeToken<RoundEnd1Token>());
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
                        if(hasNeighborAtLabel(label) && neighborAtLabel(label).state==State::Finish) {
                            RectangleParticle particle = neighborAtLabel(label);
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
                        neighborAtLabel(buildDirReverse).putToken(std::make_shared<RoundEnd2Token>());
                    }
                }
            }
            //TODO: if can go "down" again this is bad, go back
            if(countTokens<SToken>()>0 )
            {
                if( countTokens<RoundEnd2Token>()>0)
                {
                qDebug()<<"sholder got RE2";
                   buildDir = (buildDirReverse+3)%6;
                   takeToken<RoundEnd2Token>();
                }
                else if (buildDir>-1 && hasNeighborAtLabel(buildDir) && neighborAtLabel(buildDir).state==State::Finish){
                    neighborAtLabel(buildDir).putToken(takeToken<SToken>());
                }

            }
            else if(countTokens<RoundEnd2Token>()>0)
            {
                qDebug()<<"has RE2x"<<countTokens<RoundEnd2Token>();
                if( hasNeighborInState({State::Seed})){
                    int seedLabel= labelOfFirstNeighborInState({State::Seed},0);
                    neighborAtLabel(seedLabel).putToken( takeToken<RoundEnd2Token>());
                }
                else{

                    auto fillPropCheck2 = [&](const RectangleParticle& p) {
                        return   isContracted() &&
                                (p.state == State::Seed || p.state == State::Finish) &&
                                p.fillDir2>=0 &&
                                pointsAtMe(p, (p.fillDir2));
                    };
                    for(int label = 0; label < 6; label++) {
                        if(hasNeighborAtLabel(label)) {
                            RectangleParticle particle = neighborAtLabel(label);
                            if(fillPropCheck2(particle)) {
                                qDebug()<<"fillDir2 from direction: "<<label<<"my fillDir: "<<fillDir1<<","<<fillDir2;
                                neighborAtLabel(label).putToken(takeToken<RoundEnd2Token>());
                                return;
                            }

                        }
                    }
                }


                // neighborAtLabel(buildDir).putToken(takeToken<SToken>());
                //TODO only pass SToken when rectangle filled in ie receive RoundEndToken, also only build then
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
        else if (countTokens<PC1Token>()>0)
            return 0x0000ff;//blue
        else if (countTokens<PC0Token>()>0)
            return 0x9999ff;//violet
        else if (countTokens<RoundEnd1Token>()>0)
            return 0x999999;//grey
        else if (countTokens<RoundEnd2Token>()>0)
            return 0x8B008B;//dark purple
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

RectangleParticle& RectangleParticle::neighborAtLabel(int label) const
{
    return AmoebotParticle::neighborAtLabel<RectangleParticle>(label);
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

    return labelOfFirstNeighborWithProperty<RectangleParticle>(propertyCheck, startLabel);
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

    return labelOfFirstNeighborWithProperty<RectangleParticle>(propertyCheck);
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
        if(hasNeighborAtLabel(label)) {
            RectangleParticle particle = neighborAtLabel(label);
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
        if(hasNeighborAtLabel(label)) {
            RectangleParticle particle = neighborAtLabel(label);
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
        qDebug()<<"count: "<<countTokens<SToken>();
        for(int label = 0; label < 6; label++) {
            if(hasNeighborAtLabel(label)) {
                RectangleParticle particle = neighborAtLabel(label);
                if(buildPropCheck(particle)) {
                    qDebug()<<"before: "<<neighborAtLabel(label).countTokens<SToken>();
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
    if(neighborAtLabel(constructionDir).state == State::Seed) {
        constructionDir = (constructionDir + 1) % 6;
    } else {
        constructionDir = (constructionDir + 2) % 6;
    }

    if(hasNeighborAtLabel(constructionDir) && neighborAtLabel(constructionDir).state == State::Finish) {
        constructionDir = (constructionDir + 1) % 6;
    }
}


void RectangleParticle::updateMoveDir()
{
    moveDir = labelOfFirstNeighborInState({State::Seed, State::Finish});
    while(hasNeighborAtLabel(moveDir) && (neighborAtLabel(moveDir).state == State::Seed || neighborAtLabel(moveDir).state == State::Finish)) {
        moveDir = (moveDir + 5) % 6;
    }
}
bool RectangleParticle::hasTailFollower() const
{
    auto propertyCheck = [&](const RectangleParticle& p) {
        return  p.state == State::Follow &&
                pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
    };
    return labelOfFirstNeighborWithProperty<RectangleParticle>(propertyCheck) != -1;
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
