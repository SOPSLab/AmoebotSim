
#include <algorithm>
#include <cmath>
#include <deque>
#include <set>

#include "alg/leaderelectiondemo.h"
#include "sim/particle.h"
#include "sim/system.h"
#include <QDebug>


namespace LeaderElectionDemo
{
LeaderElectionDemoFlag::LeaderElectionDemoFlag()
    : followIndicator(false),
      Lnumber(-1),
      NumFinishedNeighbors(0),
      leadComplaint(false),
      seedBound(false),
      block(false),
      tokenValue(0),
      activeTokenValue(0),
      subPhase(-1),
      tokenD1(0),
      tokenD2(0),
      tokenD3(0),
      tokenCurrentDir(-1),
      ownTokenValue(-1),
      buildBorder(false),
      xT(-3),
      yT(-3),
      xTAccept(false),
      yTAccept(false),
      changeToken(false),
      handleVectors(false)

{
}

LeaderElectionDemoFlag::LeaderElectionDemoFlag(const LeaderElectionDemoFlag& other)
    : Flag(other),
      phase(other.phase),
      contractDir(other.contractDir),
      followIndicator(other.followIndicator),
      Lnumber(other.Lnumber),
      NumFinishedNeighbors(other.NumFinishedNeighbors),
      leadComplaint(other.leadComplaint),
      seedBound(other.seedBound),
      block(other.block),
      tokenValue(other.tokenValue),
      activeTokenValue(other.activeTokenValue),
      subPhase(other.subPhase),
      tokenD1(other.tokenD1),
      tokenD2(other.tokenD2),
      tokenD3(other.tokenD3),
      tokenCurrentDir(other.tokenCurrentDir),
      ownTokenValue(other.ownTokenValue),
      buildBorder(other.buildBorder),
      xT(other.xT),
      yT(other.yT),
      xTAccept(other.xTAccept),
      yTAccept(other.yTAccept),
      changeToken(other.changeToken),
      handleVectors(other.handleVectors)
{
}

LeaderElectionDemo::LeaderElectionDemo(const Phase _phase)
{
    setPhase(_phase);
    downDir = -1;
    Lnumber = -1;
    leftDir = -1;
    rightDir = -1;
    NumFinishedNeighbors = 0;
    reachedSeedBound = false;
    hasComplained = false;
    pullDir = -1;
    holdCount = 0;
    startedOffSurface = false;
    parentStage = -1;
    childStage=  -1;
    superLeader= false;
    ownTokenValue = -1;
    hasLost = false;
    borderPasses = 0;
    parentChanged = false;
    id = -1;
}

LeaderElectionDemo::LeaderElectionDemo(const LeaderElectionDemo& other)
    : AlgorithmWithFlags(other),
      phase(other.phase),
      followDir(other.followDir),
      downDir(other.downDir),
      Lnumber(other.Lnumber),
      leftDir(other.leftDir),
      rightDir(other.rightDir),
      NumFinishedNeighbors(other.NumFinishedNeighbors),
      reachedSeedBound(other.reachedSeedBound),
      hasComplained(other.hasComplained),
      pullDir(other.pullDir),
      holdCount(other.holdCount),
      startedOffSurface(other.startedOffSurface),
      parentStage(other.parentStage),
      childStage(other.childStage),
      ownTokenValue(other.ownTokenValue),
      hasLost(other.hasLost),
      borderPasses(other.borderPasses),
      parentChanged(other.parentChanged),
      id(other.id)
{
}

LeaderElectionDemo::~LeaderElectionDemo()
{
}

std::shared_ptr<System> LeaderElectionDemo::instance()
{
    std::shared_ptr<System> system = std::make_shared<System>();
    std::deque<Node> orderedSurface;
    std::set<Node> occupied;
    Node pos;
    int lastOffset = 0;
   int numParticles = 7;

    // begin hexagon structure
    int offset =0;
    int itercount =0;
    while(system->size() < 12) {
        system->insert(Particle(std::make_shared<LeaderElectionDemo>(Phase::Static), randDir(), pos));
        occupied.insert(pos);

        orderedSurface.push_back(pos);
        if(itercount%2 ==0)
            offset--;
        if(offset<0) offset = 5;
        lastOffset = offset;
        pos = pos.nodeInDir(offset);
        itercount++;
    }//end hexagon structure

    lastOffset = 0;
    int yMax= numParticles;

    std::set<Node> candidates;
    int count = 0;
    for(auto it = orderedSurface.begin(); it != orderedSurface.end(); ++it) {
        if(count >= sqrt(numParticles)) {
            break;
        }
        count++;

        for(int dir = 1; dir <= 2; dir++) {
            const Node node = it->nodeInDir(dir);
            if(occupied.find(node) == occupied.end()) {
                candidates.insert(node);
                occupied.insert(node);
            }
        }
    }
    yMax=0;
    int numNonStaticParticles = 0;
    //begin surround hexagon
    itercount = 0;
    offset = 0;
    int idCounter = 0;
    int newNumParticles = 18;
    bool initSide = true;
    while(numNonStaticParticles < newNumParticles) {
        if(candidates.empty()) {
            return system;
        }

        std::set<Node> nextCandidates;
        for(auto it = candidates.begin(); it != candidates.end() && numNonStaticParticles < newNumParticles; ++it) {
             std::shared_ptr<LeaderElectionDemo> newParticle= std::make_shared<LeaderElectionDemo>(Phase::Inactive);
            newParticle->id = idCounter;
            system->insert(Particle(newParticle, randDir(), *it));
            numNonStaticParticles++;
            if(!initSide && (itercount-1)%3 ==0)
                offset--;
            else if (initSide && itercount%2 ==0)
            {
                offset--;
                initSide = false;
            }
            if(offset<0) offset = 5;
            lastOffset = offset;
            const Node node = it->nodeInDir(offset);
            if(occupied.find(node) == occupied.end()) {
                nextCandidates.insert(node);
                occupied.insert(node);

                if(node.y > yMax) {
                    yMax = node.y;
                }

            }
            idCounter++;
        }
        nextCandidates.swap(candidates);
        itercount++;
    }
    //end surround hexagon

    return system;
}


Movement LeaderElectionDemo::execute()
{



    updateNeighborStages();

    if(phase == Phase::Lead || phase == Phase::retiredLeader)
        unsetFollowIndicator();

    if(isExpanded()) {
        clearHeldToken();
        clearVectorToken();
        if(phase == Phase::Lead)
        {
            if(phase == Phase::Lead && hasNeighborInPhase(Phase::Border))
            {
                borderPasses++;
            }

            if(hasNeighborInPhase(Phase::Inactive) || tailReceivesFollowIndicator()) {
                return Movement(MovementType::HandoverContract, tailContractionLabel());
            } else {

                return Movement(MovementType::Contract, tailContractionLabel());
            }
        }
        if(phase==Phase::Follow)
        {
            setFollowIndicatorLabel(followDir);
        }

        int activeFollowers = 0;
        for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
            auto label = *it;
            if(inFlags[label] != nullptr && !neighborIsInPhase(label,Phase::Static)) {
                if(inFlags[label]->followIndicator) {
                    activeFollowers++;
                }
            }
        }
        if( activeFollowers<2 && outFlags[0].block == true)//only if making a change
        {
            for(int label = 0; label<10; label++)
            {
                outFlags[label].block =false;
            }
            return Movement(MovementType::Idle);
        }

        setFollowIndicatorLabel(followDir);
        superLeader = isSuperLeader();//to know when contracted
        if(hasNeighborInPhase(Phase::Inactive) || tailReceivesFollowIndicator()) {

            if(phase==Phase::Hold && isSuperLeader())
            {
                setPhase(Phase::Normal);
                headMarkColor=0xFFA500;

            }
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        } else {
            if(phase==Phase::Hold && isSuperLeader())
            {
                setPhase(Phase::Normal);
            }
            return Movement(MovementType::Contract, tailContractionLabel());
        }
    } else {
        //particle is contracted
        if(phase!=Phase::Inactive && phase!=Phase::Static && phase!=Phase::Border)
        {
            setBlock();
            if(hasNeighborInPhase(Phase::Static))
            {

                handleElectionTokens();
                if(phase == Phase::Border)
                    return Movement(MovementType::Idle);
            }

            if(hasNeighborInPhase(Phase::retiredLeader) && phase!=Phase::Lead && phase!=Phase::retiredLeader) {
                setPhase(Phase::Lead);
                downDir= getDownDir();

                if(downDir == -1)
                    return Movement(MovementType::Idle);
                else downDir=labelToDir(downDir);

                Lnumber = getLnumber();
                setLayerNumber(Lnumber);
                unsetFollowIndicator();
                setPhase(Phase::Lead);

            }
        }
        if(phase == Phase::Inactive) {
            if(hasNeighborInPhase(Phase::Static)) {
                setPhase(Phase::Normal);
                startedOffSurface = false;

                downDir= getDownDir();
                if(downDir == -1)
                    return Movement(MovementType::Idle);
                else downDir=labelToDir(downDir);

                Lnumber = getLnumber();
                setLayerNumber(Lnumber);

                return Movement(MovementType::Idle);
            }
            startedOffSurface = true;
            auto label = firstNeighborInPhase(Phase::Wait);
            if(hasNeighborInPhase(Phase::Normal))
            {
                label = firstNeighborInPhase(Phase::Normal);
            }
            if(label != -1) {
                setPhase(Phase::Wait);
                followDir = labelToDir(label);
                setFollowIndicatorLabel(followDir);
                headMarkDir = followDir;

                setPhase(Phase::Wait);
                return Movement(MovementType::Idle);
            }

            return Movement(MovementType::Empty);

        }

        else if(phase!=Phase::Static && phase!=Phase::Border)
        {
            //layering block
            if(hasNeighborInPhase(Phase::Static))
            {
                downDir= getDownDir();

                if(downDir == -1)
                {
                    return Movement(MovementType::Idle);
                }
                else downDir=labelToDir(downDir);

                Lnumber = getLnumber();
                setLayerNumber(Lnumber);
                int moveDir = getMoveDir();


                if (neighborIsInPhase(moveDir, Phase::Border) || neighborIsInPhaseandLayer(moveDir, Phase::retiredLeader, Lnumber)|| reachedSeedBound)//??aya checke Lnumber nemikhad? ghanunan na age movedir dorost amal karde bashe
                {
                    setPhase(Phase::retiredLeader);
                    getLeftDir();
                    Q_ASSERT(leftDir>=0 && leftDir<=5);
                    NumFinishedNeighbors = CountFinishedSides(leftDir, rightDir);
                    setNumFinishedNeighbors(NumFinishedNeighbors);
                    headMarkDir = downDir;//-1;

                    setPhase(Phase::retiredLeader);
                    if(reachedSeedBound)
                    {
                        int seedBoundFlagDir= (downDir+3)%6;
                        while(neighborIsInPhase(seedBoundFlagDir,Phase::Static) ||neighborIsInPhase(seedBoundFlagDir,Phase::retiredLeader) )
                        {
                            seedBoundFlagDir = (seedBoundFlagDir+1)%6;
                        }
                        outFlags[seedBoundFlagDir].seedBound = true;
                    }
                    return Movement(MovementType::Idle);
                }
            }
            else
            {
                if((hasNeighborInPhase(Phase::Lead)||hasNeighborInPhase(Phase::Follow))&& (phase == Phase::Normal ||phase==Phase::Send || phase == Phase::Hold || phase==Phase::Wait))
                {
                    auto label = std::max(firstNeighborInPhase(Phase::Follow), firstNeighborInPhase(Phase::Lead));
                    if(label != -1) {
                        setPhase(Phase::Follow);
                        followDir = labelToDir(label);
                        setFollowIndicatorLabel(followDir);
                        headMarkDir = followDir;

                        setPhase(Phase::Follow);
                        return Movement(MovementType::Idle);
                    }
                }
            }
            //phase changing block
            if(phase == Phase::Normal)
            {
                if(hasNeighborInPhase(Phase::Static))
                {
                    auto label = getMoveDir();
                    if(label != -1) {

                        followDir = labelToDir(label);
                        setFollowIndicatorLabel(followDir);
                        headMarkDir = followDir;
                    }
                }

                for(int label = 0;label<10;label++)
                {
                    if((neighborIsInPhase(label,Phase::Send) ) && inFlags[label]!=nullptr && inFlags[label]->followIndicator)
                    {
                        holdCount++;
                        childStage = 0;
                    }

                }

                if(holdCount>0)
                {
                    holdCount--;
                    setPhase(Phase::Hold);
                }
                else if(!hasComplained && startedOffSurface && !neighborIsInPhase(headMarkDir,Phase::Send))
                {
                    setPhase(Phase::Hold);
                    hasComplained = true;
                }

            }
            else if (phase==Phase::Wait)
            {
                if(!hasComplained && !hasNeighborInPhase(Phase::Inactive) )
                {
                    setPhase(Phase::Hold);
                    hasComplained = true;
                }

                else if(hasNeighborInPhase(Phase::Static))
                {
                    setPhase(Phase::Normal);
                }

            }
            else if(phase == Phase:: Hold)
            {
                if(hasNeighborInPhase(Phase::Static)&&!(parentActivated()||neighborIsInPhase(headMarkDir,Phase::Inactive)))
                {
                    int moveDir = getMoveDir();
                    setContractDir(moveDir);
                    headMarkDir = moveDir;
                    followDir = headMarkDir;
                    auto temp = dirToHeadLabelAfterExpansion(followDir, moveDir);
                    setFollowIndicatorLabel(temp);

                    if(!(inFlags[moveDir]!=nullptr && (inFlags[moveDir]->isContracted())) )
                    {
                        clearHeldToken();
                        return Movement(MovementType::Expand, moveDir);
                    }
                }
                bool childrenNormal = true;
                for(int label = 0;label<10;label++)
                {
                    if(inFlags[label]!=nullptr && inFlags[label]->followIndicator  && (neighborIsInPhase(label,Phase::Send)))
                        childrenNormal = false;
                }

                if(neighborIsInPhase(headMarkDir,Phase::Normal))
                {
                    if(childrenNormal || childStage!=0)
                    {
                        setPhase(Phase::Send);

                    }
                }
            }
            else if(phase == Phase::Send)
            {
                if(parentStage == 1)
                {
                    setPhase(Phase::Normal);
                    parentStage = -1;
                }
                else if(!parentActivated())
                {
                    parentStage = -1;
                    setPhase(Phase::Hold);
                }
            }
            else if(phase == Phase::Lead)
            {

                downDir= getDownDir();
                if(downDir == -1)
                {
                    setPhase(Phase::Follow);
                    return Movement(MovementType::Idle);
                }
                else
                    downDir=labelToDir(downDir);

                //check for tunnel against border, if necessary/possible, fillit
                if(hasNeighborInPhase(Phase::retiredLeader)&& hasNeighborInPhase(Phase::Border))
                {
                    int borderStart = firstNeighborInPhase(Phase::Border);
                    /*  int forward=  (borderStart+1)%6;
                    if(inFlags[forward]==nullptr && hasNeighborInPhase(Phase::retiredLeader))
                    {
                        Lnumber = getLnumber();
                        setLayerNumber(Lnumber);
                        int moveDir = forward;
                        setContractDir(moveDir);
                        headMarkDir = moveDir;
                        downDir = moveDir;
                        setPhase(Phase::Lead);
                        clearHeldToken();
                        qDebug()<<"fill fwd hole";
                        headMarkColor = 0x551A8B;
                        return Movement(MovementType::Expand, moveDir);
                    }
                    int back=  (borderStart+5)%6;
                    if(inFlags[back]==nullptr &&hasNeighborInPhase(Phase::retiredLeader))// neighborIsInPhase((borderStart+2)%6,Phase::retiredLeader))
                    {
                        Lnumber = getLnumber();
                        setLayerNumber(Lnumber);
                        int moveDir = back;
                        setContractDir(moveDir);
                        headMarkDir = moveDir;
                        downDir = moveDir;
                        setPhase(Phase::Lead);
                        clearHeldToken();
                        qDebug()<<"fill back hole";
                        return Movement(MovementType::Expand, moveDir);
                    }*/
                }
                Lnumber = getLnumber();
                setLayerNumber(Lnumber);
                int moveDir = getMoveDir();

                if (neighborIsInPhase(moveDir, Phase::Border)|| neighborIsInPhaseandLayer(moveDir, Phase::retiredLeader, Lnumber) ||(hasNeighborInPhase(Phase::Border)&& borderPasses>1))//??aya checke Lnumber nemikhad? ghanunan na age movedir dorost amal karde bashe
                {

                    setPhase(Phase::retiredLeader);
                    getLeftDir();
                    Q_ASSERT(leftDir>=0 && leftDir<=5);
                    NumFinishedNeighbors = CountFinishedSides(leftDir, rightDir);
                    setNumFinishedNeighbors(NumFinishedNeighbors);
                    headMarkDir = downDir;//-1;

                    setPhase(Phase::retiredLeader);

                    return Movement(MovementType::Idle);
                }


                setContractDir(moveDir);
                headMarkDir = downDir;
                setPhase(Phase::Lead);
                clearHeldToken();

                return Movement(MovementType::Expand, moveDir);

            }
            else if(phase == Phase::Follow) {
                Q_ASSERT(inFlags[followDir] != nullptr);
                if(hasNeighborInPhase(Phase::Static)) {
                    setPhase(Phase::Lead);
                    downDir= getDownDir();

                    if(downDir == -1)
                        return Movement(MovementType::Idle);
                    else downDir=labelToDir(downDir);

                    Lnumber = getLnumber();
                    setLayerNumber(Lnumber);
                    unsetFollowIndicator();

                    setPhase(Phase::Lead);
                    return Movement(MovementType::Idle);
                }
                if(hasNeighborInPhase(Phase::retiredLeader)) {


                    setPhase(Phase::Lead);
                    downDir= getDownDir();

                    if(downDir == -1)
                        return Movement(MovementType::Idle);
                    else downDir=labelToDir(downDir);

                    Lnumber = getLnumber();
                    setLayerNumber(Lnumber);
                    unsetFollowIndicator();

                    setPhase(Phase::Lead);

                    return Movement(MovementType::Idle);
                }
                if(inFlags[followDir]->isExpanded() && !inFlags[followDir]->fromHead) {
                    int expansionDir = followDir;
                    setContractDir(expansionDir);
                    followDir = updatedFollowDir();
                    headMarkDir = followDir;
                    auto temp = dirToHeadLabelAfterExpansion(followDir, expansionDir);
                    Q_ASSERT(labelToDirAfterExpansion(temp, expansionDir) == followDir);
                    setFollowIndicatorLabel(temp);
                    return Movement(MovementType::Expand, expansionDir);
                }
                return Movement(MovementType::Idle);

            }

            else if(phase == Phase::retiredLeader )
            {
                for(int label= 0; label<10;label++)
                {
                    if(inFlags[label]!=nullptr && inFlags[label]->buildBorder)
                    {
                        setPhase(Phase::Border);
                        int borderBuildDir = (headMarkDir+3)%6;
                        while(neighborIsInPhase(borderBuildDir,Phase::Static))
                            borderBuildDir = (borderBuildDir+1)%6;
                        outFlags[borderBuildDir].buildBorder= true;
                        qDebug()<<"trying to border?";
                        return Movement(MovementType::Idle);
                    }
                }
                getLeftDir();
                Q_ASSERT(leftDir >=0 && leftDir<=5);
                NumFinishedNeighbors = CountFinishedSides(leftDir, rightDir);
                setNumFinishedNeighbors(NumFinishedNeighbors);
                Q_ASSERT(NumFinishedNeighbors>=0 && NumFinishedNeighbors<=2);
                if(NumFinishedNeighbors==2)
                {
                    if(reachedSeedBound)
                    {
                        int seedBoundFlagDir= (downDir+3)%6;
                        int tries = 0;
                        while(neighborIsInPhase(seedBoundFlagDir,Phase::Static) ||neighborIsInPhase(seedBoundFlagDir,Phase::retiredLeader) && tries<10)
                        {
                            seedBoundFlagDir = (seedBoundFlagDir+1)%6;
                            tries++;
                        }
                        outFlags[seedBoundFlagDir].seedBound = true;
                    }
                }
                setPhase(Phase::retiredLeader);
                for(int label =0; label<10;label++)
                {
                    if(inFlags[label]!=nullptr && inFlags[label]->seedBound && !reachedSeedBound)
                    {
                        qDebug()<<"retired stuck to bound";
                    }
                }
                return Movement(MovementType::Idle);

            }
            //motion block
            if(phase == Phase::Normal || phase==Phase::Hold || phase == Phase::Wait || phase==Phase::Send )
            {
                updateNeighborStages();
                setFollowIndicatorLabel(followDir);
                if(followDir>-1 && followDir<10 && headMarkDir>-1&& !hasNeighborInPhase(Phase::Inactive))
                {
                    if(inFlags[followDir]!=nullptr  && inFlags[followDir]->isExpanded() && !inFlags[followDir]->fromHead && (!inFlags[followDir]->block || !hasNeighborInPhase(Phase::Static)) ) {
                        int expansionDir = followDir;

                        setContractDir(expansionDir);
                        followDir = updatedFollowDir();
                        headMarkDir = followDir;
                        auto temp = dirToHeadLabelAfterExpansion(followDir, expansionDir);
                        Q_ASSERT(labelToDirAfterExpansion(temp, expansionDir) == followDir);
                        setFollowIndicatorLabel(temp);
                        clearHeldToken();
                        return Movement(MovementType::Expand, expansionDir);
                    }
                }



                return Movement(MovementType::Idle);
            }
        }
    }
    if(phase == Phase::Lead)
        qDebug()<<"lead empty";

    if(phase==Phase::retiredLeader)
        qDebug()<<"retired empty";
    return Movement(MovementType::Empty);
}

std::shared_ptr<Algorithm> LeaderElectionDemo::clone()
{
    return std::make_shared<LeaderElectionDemo>(*this);
}

bool LeaderElectionDemo::isDeterministic() const
{
    return true;
}

void LeaderElectionDemo::setPhase(const Phase _phase)
{
    phase = _phase;
    if(headMarkColor== 0xFFFF00) return;
    if(phase == Phase::Normal)
    {
        headMarkColor = 0xff0000;
        tailMarkColor = 0xff0000;
    }
    else if (phase==Phase::Wait)
    {
        headMarkColor = 0x0000ff;
        tailMarkColor = 0x0000ff;
    }
    else if(phase == Phase:: Hold)
    {
        headMarkColor = 0xFF69B4;
        tailMarkColor = 0xFF69B4;
    }
    else if(phase == Phase::Send)
    {
        headMarkColor = 0x00ff00;
        tailMarkColor = 0x00ff00;
    }

    else if(phase == Phase::retiredLeader){
        if(NumFinishedNeighbors ==2)
        {
            headMarkColor = 0x00ff00;
            tailMarkColor = 0x00ff00;
        }
    }
    else if(phase == Phase::Seed)
    {
        headMarkColor = 0xFF69B4;
        tailMarkColor = 0xFF69B4;
    }

    else if(phase == Phase::Inactive) {
        headMarkColor = -1;
        tailMarkColor = -1;
    } else  if(phase == Phase::Border)
    {
        headMarkColor = 0xd3d3d3;
        tailMarkColor = 0xd3d3d3;

    }
    else  if(phase == Phase::Follow)
    {
        headMarkColor = 0xFFA500;
        tailMarkColor = 0xFFA500;
    }
    else

    { // phase == Phase::Static
        headMarkColor = 0x000000;
        tailMarkColor = 0x000000;
    }
    if(phase != Phase::Static)
    {
        if(ownTokenValue == -1)
        {
            if(outFlags[0].subPhase == -1)
            {
                headMarkColor = 0x660000;
                tailMarkColor = 0xff0000;
            }
            else if(outFlags[0].subPhase == 0)
            {
                headMarkColor = 0x1F4C0F;
                tailMarkColor = 0xff0000;
            }
            else if(outFlags[0].subPhase == 1)
            {
                headMarkColor = 0x002966;
                tailMarkColor = 0xff0000;
            }
            else if(outFlags[0].subPhase == 2)
            {
                headMarkColor = 0xE6B800;
                tailMarkColor = 0xff0000;
            }
        }
        else if(ownTokenValue == 0)
        {
            if(outFlags[0].subPhase == -1)
            {
                headMarkColor = 0xff0000;
                tailMarkColor = 0xff0000;
            }
            else if(outFlags[0].subPhase == 0)
            {
                headMarkColor = 0x66FF33;
                tailMarkColor = 0xff0000;
            }
            else if(outFlags[0].subPhase == 1)
            {
                headMarkColor = 0x0066FF;
                tailMarkColor = 0xff0000;
            }
            else if(outFlags[0].subPhase == 2)
            {
                headMarkColor = 0xFFFF66;
                tailMarkColor = 0xff0000;
            }
        }
    }


    for(int label = 0; label < 10; label++) {
        outFlags[label].phase = phase;
    }
}

bool LeaderElectionDemo::neighborIsInPhase(const int label, const Phase _phase) const
{
    Q_ASSERT(0 <= label && label <= 9);
    return (inFlags[label] != nullptr && inFlags[label]->phase == _phase);
}

bool LeaderElectionDemo::neighborIsInPhaseandLayer(const int label, const Phase _phase, const int L) const
{
    Q_ASSERT(isContracted());

    return (inFlags[label] != nullptr && inFlags[label]->phase == _phase && inFlags[label]->Lnumber == L);
}

int LeaderElectionDemo::firstNeighborInPhase(const Phase _phase) const
{

    for(int label = 0; label < 10; label++) {
        if(neighborIsInPhase(label, _phase)) {
            return label;
        }
    }
    return -1;
}

int LeaderElectionDemo::firstNeighborInPhaseandLayer(const Phase _phase, const int L) const
{
    for(int label = 0; label < 10; label++) {
        if(neighborIsInPhaseandLayer(label, _phase, L)){
            return label;
        }
    }
    return -1;
}
bool LeaderElectionDemo::hasNeighborInPhase(const Phase _phase) const
{
    return (firstNeighborInPhase(_phase) != -1);
}

void LeaderElectionDemo::setContractDir(const int contractDir)
{
    for(int label = 0; label < 10; label++) {
        outFlags[label].contractDir = contractDir;
    }
}

int LeaderElectionDemo::updatedFollowDir() const
{
    int contractDir = inFlags[followDir]->contractDir;
    int offset = (followDir - inFlags[followDir]->dir + 9) % 6;
    int tempFollowDir = (contractDir + offset) % 6;
    Q_ASSERT(0 <= tempFollowDir && tempFollowDir <= 5);
    return tempFollowDir;
}

void LeaderElectionDemo::unsetFollowIndicator()
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = false;
    }
}

void LeaderElectionDemo::setFollowIndicatorLabel(const int label)
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = (i == label);
    }
}

bool LeaderElectionDemo::tailReceivesFollowIndicator() const
{
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        auto label = *it;
        if(inFlags[label] != nullptr) {
            if(inFlags[label]->followIndicator) {
                return true;
            }
        }
    }
    return false;
}

int LeaderElectionDemo::getMoveDir() const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(Lnumber!=-1);
    int label= -1;
    if(Lnumber %2 == 0)
    {
        label = firstNeighborInPhase(Phase::Static);
        if(label != -1)
        {
            label = (label+5)%6;
            while (neighborIsInPhase(label, Phase::Static)){
                label = (label+5)%6;
            }
        }
        else
        {
            label = firstNeighborInPhaseandLayer(Phase::retiredLeader, (Lnumber+1)%2);
            label = (label+5)%6;
            while (neighborIsInPhaseandLayer(label, Phase::retiredLeader, (Lnumber+1)%2)){

                label = (label+5)%6;
            }
        }
    }
    else
    {
        label = firstNeighborInPhase(Phase::Static);
        if(label != -1)
        {
            label = (label+1)%6;
            while (neighborIsInPhase(label, Phase::Static)){
                label = (label+1)%6;
            }
        }
        else
        {
            label = firstNeighborInPhaseandLayer(Phase::retiredLeader, (Lnumber+1)%2);
            label = (label+1)%6;
            while (neighborIsInPhaseandLayer(label, Phase::retiredLeader, (Lnumber+1)%2)){
                label = (label+1)%6;
            }
        }
    }
    Q_ASSERT(0 <= label && label <= 5);
    return labelToDir(label);
}

int LeaderElectionDemo::getDownDir() const
{
    Q_ASSERT(isContracted());
    //Q_ASSERT(phase==Phase::Lead);

    int label= -1;
    if(hasNeighborInPhase(Phase::Static)) {
        label = firstNeighborInPhase(Phase::Static);

    }
    else
    {
        int ZeroLayerGreens=0;
        int OneLayerGreens = 0;
        if(hasNeighborInPhase(Phase::retiredLeader))
        {
            ZeroLayerGreens = countGreenNeighbors(Phase::retiredLeader, 0);
            OneLayerGreens =  countGreenNeighbors(Phase::retiredLeader, 1);
            if(ZeroLayerGreens>0 && OneLayerGreens==0) //down is obviously towards that only layer of retired leaders
            {
                label= firstNeighborInPhaseandLayer(Phase::retiredLeader, 0);
                Q_ASSERT(label>=0 && label<=5);
            }
            else if(ZeroLayerGreens==0 && OneLayerGreens>0)
            {
                label = firstNeighborInPhaseandLayer(Phase::retiredLeader, 1);
                Q_ASSERT(label>=0 && label<=5);

            }else //from each layer there are some retireds. So we need to decide based on f
            {
                if(countRetiredareFinished(0) == ZeroLayerGreens)
                {
                    label= firstNeighborInPhaseandLayer(Phase::retiredLeader, 0);
                }
                else if(countRetiredareFinished(1) == OneLayerGreens)
                {
                    label = firstNeighborInPhaseandLayer(Phase::retiredLeader, 1);
                }

                else label= -1;
            }
        }
    }
    return label;
}

int LeaderElectionDemo::getLnumber() const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(downDir != -1);
    Q_ASSERT(inFlags[downDir] != nullptr);
    if(inFlags[downDir] != nullptr)
    {
        if(inFlags[downDir]->phase == Phase::Static)
            return 0;
        else
            return (inFlags[downDir]->Lnumber +1)%2;

    }
}

void LeaderElectionDemo::getLeftDir()
{
    Q_ASSERT(isContracted());
    Q_ASSERT(phase == Phase::retiredLeader);
    Q_ASSERT(downDir != -1);

    int label= downDir;
    int leftside= -1, rightside= -1;
    if(neighborIsInPhase(label, Phase::Static))
    {
        label = (label+5)%6;
        while (neighborIsInPhase(label, Phase::Static)){
            label = (label+5)%6;
        }
        leftside= label;
    }
    else
    {
        label= downDir;
        if(neighborIsInPhase(label, Phase::retiredLeader))
        {
            label = (label+5)%6;
            while (neighborIsInPhaseandLayer(label, Phase::retiredLeader, (Lnumber+1)%2)){
                label = (label+5)%6;
            }
            leftside= label;
        }
    }

    label= downDir;
    if(neighborIsInPhase(label, Phase::Static))
    {
        label = (label+1)%6;
        while (neighborIsInPhase(label, Phase::Static)){
            label = (label+1)%6;
        }
        rightside= label;
    }
    else
    {   label= downDir;
        if(neighborIsInPhase(label, Phase::retiredLeader))
        {
            label = (label+1)%6;
            while (neighborIsInPhaseandLayer(label, Phase::retiredLeader, (Lnumber+1)%2)){
                label = (label+1)%6;
            }
            rightside= label;
        }
    }

    leftDir = labelToDir(leftside);
    rightDir= labelToDir(rightside);
}

int LeaderElectionDemo::countGreenNeighbors(const Phase _phase, const int L) const
{
    int count=0;
    for(int dir = 0; dir < 10; dir++) {
        if(neighborIsInPhaseandLayer(dir, _phase, L) ) {
            count++;
        }
    }
    return count;
}
void LeaderElectionDemo::setLayerNumber(const int _Lnumber){
    Q_ASSERT(_Lnumber==0 || _Lnumber==1);
    for(int label = 0; label < 10; label++){
        outFlags[label].Lnumber= _Lnumber;
    }
}

int LeaderElectionDemo::CountFinishedSides(const int _leftDir, const int _rightDir) const
{
    int count=0;
    Q_ASSERT(_leftDir != -1);
    //I have not put leftdit into my outflags but I am accessing thme from my inflags!
    if(neighborIsInPhase(_leftDir, Phase::Border) || neighborIsInPhase(_leftDir, Phase::retiredLeader))
        count++;
    if(neighborIsInPhase(_rightDir, Phase::Border) || neighborIsInPhase(_rightDir, Phase::retiredLeader))
        count++;

    return count;
}

int LeaderElectionDemo::countRetiredareFinished(const int _Lnumber) const
{
    int countFinished =0;
    for(int dir = 0; dir < 10; dir++) {
        if(neighborIsInPhaseandLayer(dir, Phase::retiredLeader, _Lnumber) && inFlags[dir]->NumFinishedNeighbors == 2) {
            countFinished++; //number of retired leaders p has with layer number =0

        }
    }

    return countFinished;
}

void LeaderElectionDemo::setNumFinishedNeighbors(const int _NumFinishedNeighbors)
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].NumFinishedNeighbors = _NumFinishedNeighbors;
    }
}
bool LeaderElectionDemo::inFrontOfLeadC() const
{
    for(int label =0; label<10; label++)
    {
        if(inFlags[label]!=nullptr && inFlags[label]->leadComplaint)
        {
            return true;
        }

    }
    return false;

}
void LeaderElectionDemo::setLeadComplaint(bool value)
{
    for(int label = 0; label<10;label++)
    {
        outFlags[label].leadComplaint = value;
    }
    if(value)
    {
        headMarkColor =0xFF69B4;
        tailMarkColor = 0xFF69B4;
    }
    else
    {
        qDebug()<<"set lead complaint";
        headMarkColor = 0xff0000;
        tailMarkColor = 0xff0000;
    }
}
void LeaderElectionDemo::setBlock()
{

    bool block = false;

    if(hasNeighborInPhase(Phase::Static))
    {
        /* for(int label = 0; label<10; label++)
        {
            if((neighborIsInPhase(label,Phase::Wait) || neighborIsInPhase(label,Phase::Hold)|| neighborIsInPhase(label,Phase::Send)) && label!=headMarkDir && inFlags[label]!=nullptr && inFlags[label]->followIndicator)
            {
                block = true;
                break;
            }
        }*/
        auto label = firstNeighborInPhase(Phase::Static);
        int behindLabel = (label+1)%6;
        int frontLabel = label;
        while(neighborIsInPhase(frontLabel,Phase::Static))
        {
            frontLabel = (frontLabel+5)%6;
        }
        int checkLabel = (behindLabel+1)%6;
        int count = 0;
        while(checkLabel!=frontLabel)
        {
            if(inFlags[checkLabel]!=nullptr && inFlags[checkLabel]->followIndicator)
                count++;
            checkLabel = (checkLabel+1)%6;
        }
        if(count>0)
        {
            block = true;
        }

    }
    for(int label = 0; label<10; label++)
    {
        outFlags[label].block =block;

    }
}
void LeaderElectionDemo::expandedSetBlock()
{


}
bool LeaderElectionDemo::isSuperLeader()
{

    int headNeighbors = 0;
    int tailNeighbors = 0;
    auto label = firstNeighborInPhase(Phase::Static);
    if(label != -1)
    {
        label = (label+9)%10;
        while (neighborIsInPhase(label, Phase::Static)){
            label = (label+9)%10;
        }
        if(inFlags[label]!=nullptr)
            headNeighbors++;
    }
    label = firstNeighborInPhase(Phase::Static);
    if(label != -1)
    {
        label = (label+1)%10;
        if(inFlags[label]!=nullptr)
            tailNeighbors++;

    }
    if(hasNeighborInPhase(Phase::Static) && headNeighbors==0)// && tailNeighbors == 1)
        return true;
    return false;
}
void LeaderElectionDemo::updateParentStage()
{
    if(headMarkDir>-1)
    {

        if(parentStage == -1)
        {
            if(!neighborIsInPhase(headMarkDir,Phase::Hold))
                parentStage = 0;
        }
        else if(parentStage == 0)
        {
            if(neighborIsInPhase(headMarkDir,Phase::Hold))
                parentStage = 1;
        }
        else if (parentStage == 1)
        {
            if(!neighborIsInPhase(headMarkDir,Phase::Hold))
                parentStage = -1;
        }


    }
}
void LeaderElectionDemo::updateChildStage()
{
    if(childStage == -1)
    {
        bool hasHoldingChild = false;
        for(int label = 0; label<10;label++)
        {
            if(inFlags[label]!=nullptr && neighborIsInPhase(label,Phase::Hold))
            {
                hasHoldingChild = true;
            }
        }
        if(hasHoldingChild)
        {
            childStage = 0;
        }
        else
        {
            childStage = 1;
        }
    }
    //1, 0 stay until self changes, resets childStage to -1

}
void LeaderElectionDemo::updateNeighborStages()
{
    if(phase ==Phase::Send || phase==Phase::Hold)
        updateParentStage();
    else
        parentStage = -1;


    if(phase!=Phase::Hold)
        childStage = -1;
}

bool LeaderElectionDemo::parentActivated()
{
    if(neighborIsInPhase(headMarkDir,Phase::Hold) || neighborIsInPhase(headMarkDir,Phase::Send)
            ||neighborIsInPhase(headMarkDir,Phase::Wait) || neighborIsInPhase(headMarkDir,Phase::Normal))
        return true;

    return false;
}


void LeaderElectionDemo::handleElectionTokens()
{
    qDebug()<<id<<": "<<ownTokenValue;
    //step 0: setup
    auto surfaceFollower = firstNeighborInPhase(Phase::Static);
    int   surfaceParent = headMarkDir;
    if(surfaceFollower!=-1)
    {
        while(neighborIsInPhase(surfaceFollower,Phase::Static))
            surfaceFollower = (surfaceFollower+1)%6;
    }
    if( surfaceFollower<0 || surfaceParent<0 ||
            neighborIsInPhase(surfaceParent,Phase::Inactive) || neighborIsInPhase(surfaceFollower,Phase::Inactive ) ||
            inFlags[surfaceFollower] ==nullptr || inFlags[surfaceParent]==nullptr)
    {
        return;
    }
    //own = -1--> leader candidate
    //if adjacent to candidate, demote self
    if(ownTokenValue == -1)
    {
        if(inFlags[surfaceFollower]->isContracted() && inFlags[surfaceParent]->isContracted() )//don't try and eliminate self till surrounded on surface
        {
            if(inFlags[surfaceParent]->ownTokenValue == -1)
            {
                qDebug()<<"  changed to 0";
                ownTokenValue = 0;

                for(int label=0;label<10;label++)
                {
                    outFlags[label].ownTokenValue = ownTokenValue;
                }
            }
        }

    }
    //if this method is being executed, particle meets requirements to start leader election.
    int currentSubPhase = outFlags[0].subPhase;
    int tokenValue = outFlags[0].tokenValue;
    int activeTokenValue = outFlags[0].activeTokenValue;

    qDebug()<<"   subphase: "<<currentSubPhase<<" tokenval: "<<tokenValue<<" active: " <<activeTokenValue;

    if(currentSubPhase == -1)
    {
        currentSubPhase =0;
    }

    if(currentSubPhase == 0) //not started-- see if can initialize, or initialization chain has started
    {
        //candidate changes subphase to signal fs but doesn't send token
        if(ownTokenValue == -1)
        {
            currentSubPhase = 1;
        }
        //fs changes subphase + sends cover/EOS token
        if(ownTokenValue == 0 && inFlags[surfaceFollower]->subPhase == 1)
        {
            currentSubPhase = 1;
            if(inFlags[surfaceParent]->ownTokenValue == -1)//eos- parent is next candidate
            {
                tokenValue = 2;
            }
            else
            {
                tokenValue = 1;
            }
        }
    }

    else if (currentSubPhase == 1) //has been started, could be matched- else to force wait for next round
    {

        //candidate
        if(ownTokenValue == -1)
        {
            //receives own passive tokens

            //receives cover/eos token- if self is clear
            if(tokenValue == 0 && inFlags[surfaceFollower]->tokenValue>-1 &&(inFlags[surfaceParent]->tokenValue == 1 ||inFlags[surfaceParent]->tokenValue == 2))
            {
                tokenValue = -1* inFlags[surfaceParent]->tokenValue;
                qDebug()<<"candidate received token";
            }
            //switches to send if receive complete
            else if(inFlags[surfaceParent]->tokenValue <1 && tokenValue < 0 && activeTokenValue==0 && inFlags[surfaceFollower]->activeTokenValue>-1)
            {
                activeTokenValue = -1 *tokenValue;
                tokenValue = 0;//end passive token, start an active token

                qDebug()<<"candidate forwarded token";
            }
            //clear if send complete
            else if(inFlags[surfaceFollower]->tokenValue<0 && tokenValue > 0 )
            {
                tokenValue = 0;
                qDebug()<<"candidate cleared";
            }

            //receives other's active tokens- give up candidacy
            if(activeTokenValue == 0 && inFlags[surfaceFollower]->activeTokenValue>-1 &&  inFlags[surfaceParent]->activeTokenValue >0 )
            {
                activeTokenValue = -1 * inFlags[surfaceParent]->activeTokenValue;
            }
            else if (activeTokenValue>0 && inFlags[surfaceFollower]->activeTokenValue <0)
            {
                activeTokenValue = 0;
            }
            //match with received token- clear it and go to subphase 2
            else if(activeTokenValue == -1 && inFlags[surfaceParent]->activeTokenValue == 0)
            {
                activeTokenValue = 0;
                //   ownTokenValue = 0;
                currentSubPhase = 2;
            }
            //match with eos token, clear it and go to subphase 3 (finished)
            else if(activeTokenValue == -2 && inFlags[surfaceParent]->activeTokenValue == 0)
            {
                activeTokenValue = 0;
                currentSubPhase = 4;
            }
        }

        else if(ownTokenValue == 0)
        {
            //passive block  - 1s and 2s

            //receives cover/eos token- if self is clear
            if(tokenValue == 0 && inFlags[surfaceFollower]->tokenValue>-1 && (inFlags[surfaceParent]->tokenValue == 1 ||inFlags[surfaceParent]->tokenValue == 2))
            {
                tokenValue = -1* inFlags[surfaceParent]->tokenValue;
                qDebug()<<"noncandidate received: "+tokenValue;
            }
            //switches to send if receive complete
            else if(inFlags[surfaceParent]->tokenValue <1 && (tokenValue == -1 || tokenValue == -2))
            {
                tokenValue = -1 * tokenValue;
                qDebug()<<"noncandidate sending: "+tokenValue;
            }
            //clear if send complete
            else if(inFlags[surfaceFollower]->tokenValue<0 && (tokenValue == 1 || tokenValue == 2) )
            {
                tokenValue = 0;
                qDebug()<<"noncandidate cleared";

            }

            //active block
            if(activeTokenValue == 0 && inFlags[surfaceFollower]->activeTokenValue>-1 &&  inFlags[surfaceParent]->activeTokenValue >0 )
            {
                activeTokenValue = -1 * inFlags[surfaceParent]-> activeTokenValue ;
            }
            else if (activeTokenValue > 0 && inFlags[surfaceFollower]->activeTokenValue <0)
            {
                activeTokenValue = 0;
            }
            //match with received token- clear it and go to subphase 2
            else if(activeTokenValue == -1 && inFlags[surfaceParent]->activeTokenValue == 0)
            {
                activeTokenValue = 0;
                currentSubPhase = 2;
            }
            //match with eos token, clear it and go to subphase 3 (finished)
            else if(activeTokenValue == -2 && inFlags[surfaceParent]->activeTokenValue == 0)
            {
                activeTokenValue = 0;
                currentSubPhase = 3;
            }
        }



    }
    else if (currentSubPhase >1 )//matched-all who are matched are not candidates/owntokenvalue = 0
    {

        if(ownTokenValue == 0)
        {
            //passive tokens- receive, pass along
            if(tokenValue == 0 && inFlags[surfaceFollower]->tokenValue>-1 && inFlags[surfaceParent]->tokenValue >0)
            {
                tokenValue = -1 * inFlags[surfaceParent]->tokenValue ;
            }
            else if (tokenValue > 0 && inFlags[surfaceFollower]->tokenValue <0)
            {
                tokenValue = 0;
            }
            //since already matched, sending received token
            else if(tokenValue < 0 && inFlags[surfaceParent]->tokenValue <1)
            {
                tokenValue = -1 * tokenValue;
            }

            //active tokens- receive, pass along
            if(activeTokenValue == 0 && inFlags[surfaceFollower]->activeTokenValue>-1 && inFlags[surfaceParent]->activeTokenValue >0)
            {
                activeTokenValue = -1 * inFlags[surfaceParent]->activeTokenValue ;
            }
            else if (activeTokenValue > 0 && inFlags[surfaceFollower]->activeTokenValue <0)
            {
                activeTokenValue = 0;
            }
            //since already matched, sending received token
            else if(activeTokenValue < 0 && inFlags[surfaceParent]->activeTokenValue <1)
            {
                activeTokenValue = -1 * activeTokenValue;
            }
            if(inFlags[surfaceFollower]->subPhase ==3 || inFlags[surfaceFollower]->subPhase ==4)
            {
                currentSubPhase = 3;
            }

        }
        else if(ownTokenValue == -1)
        {
            tokenValue = 0;
            activeTokenValue = 0;
            if(inFlags[surfaceFollower]->subPhase ==3 || inFlags[surfaceFollower]->subPhase ==4)
            {
                currentSubPhase = 4;
            }

        }
    }
    else if (currentSubPhase == 3)//finished
    {

    }
    else if(currentSubPhase == 4)//transfer
    {

    }
    for(int i =0; i<10;i++)
    {
        outFlags[i].subPhase = currentSubPhase;
        outFlags[i].tokenValue = tokenValue;
        outFlags[i].activeTokenValue = activeTokenValue;

    }
    qDebug()<<"   subphase: "<<currentSubPhase<<" tokenval: "<<tokenValue<<" active: " <<activeTokenValue;

    if(ownTokenValue == -1)
    {
        if(outFlags[0].subPhase == -1)
        {
            headMarkColor = 0x660000;
            tailMarkColor = 0xff0000;
        }
        else if(outFlags[0].subPhase == 0)
        {
            headMarkColor = 0x1F4C0F;
            tailMarkColor = 0xff0000;
        }
        else if(outFlags[0].subPhase == 1)
        {
            headMarkColor = 0x002966;
            tailMarkColor = 0xff0000;
        }
        else if(outFlags[0].subPhase == 2)
        {
            headMarkColor = 0xE6B800;
            tailMarkColor = 0xff0000;
        }
        else if(outFlags[0].subPhase == 4)
        {
            headMarkColor = 0xFF66FF;
            tailMarkColor = 0xff0000;
        }
    }
    else if(ownTokenValue == 0)
    {
        if(outFlags[0].subPhase == -1)
        {
            headMarkColor = 0xff0000;
            tailMarkColor = 0xff0000;
        }
        else if(outFlags[0].subPhase == 0)
        {
            headMarkColor = 0x66FF33;
            tailMarkColor = 0xff0000;
        }
        else if(outFlags[0].subPhase == 1)
        {
            headMarkColor = 0x0066FF;
            tailMarkColor = 0xff0000;
        }
        else if(outFlags[0].subPhase == 2)
        {
            headMarkColor = 0xFFFF66;
            tailMarkColor = 0xff0000;
        }
        else if(outFlags[0].subPhase == 3)
        {
            headMarkColor = 0xFFCCFF;
            tailMarkColor = 0xff0000;
        }
    }

}
void LeaderElectionDemo::oldHandleElectionTokens()
{
    qDebug()<<"handle election tokens";
    //step 0: setup
    auto surfaceFollower = firstNeighborInPhase(Phase::Static);
    int   surfaceParent = headMarkDir;
    if(surfaceFollower!=-1)
    {
        while(neighborIsInPhase(surfaceFollower,Phase::Static))
            surfaceFollower = (surfaceFollower+1)%6;
    }
    //step 1: set up token values to random numbers
    if(ownTokenValue== -1)
    {
        int prevOwn = ownTokenValue;
        while(prevOwn == ownTokenValue)
        {
            ownTokenValue = rand() % 8;
        }

        for(int label=0;label<10;label++)
        {
            outFlags[label].ownTokenValue = ownTokenValue;
        }
        setTokenValue(ownTokenValue);
        qDebug()<<"init: "<<ownTokenValue;

    }
    if(outFlags[0].tokenValue == -1)
    {
        setTokenValue(outFlags[0].ownTokenValue);
    }

    if(surfaceFollower != -1 && surfaceParent!=-1)
    {
        if(inFlags[surfaceFollower]!=nullptr  && inFlags[surfaceFollower]->tokenValue!=-1 && inFlags[surfaceParent]!=nullptr &&inFlags[surfaceParent]->tokenValue!=-1 )
        {

            //head
            if(outFlags[0].tokenValue == outFlags[0].ownTokenValue)
            {
                //if not changing, can be passed over
                if(inFlags[surfaceFollower]->tokenValue>outFlags[0].tokenValue && !outFlags[0].changeToken)
                {
                    setTokenValue(inFlags[surfaceFollower]->tokenValue);
                    clearVectorToken();
                }
                //equal tail touching- start delta
                else if(inFlags[surfaceFollower]->tokenValue==outFlags[0].ownTokenValue && !outFlags[0].handleVectors)
                {

                    if( !outFlags[0].changeToken)
                    {
                        //a direction hasn't been sent forward yet (can't check if direction set behind, could have been set by someone else)
                        //if this is a head behind an equal head, this condition will prevent anything from happening
                        if(inFlags[surfaceParent]->tokenCurrentDir == -1)
                        {
                            for(int label = 0; label<10;label++)
                            {
                                outFlags[label].changeToken =  true;
                            }
                            //change token
                            ownTokenValue = rand() % 8;
                            qDebug()<<"token: "<<outFlags[0].ownTokenValue<<" changed to "<<ownTokenValue;
                            for(int label=0;label<10;label++)
                            {
                                outFlags[label].ownTokenValue = ownTokenValue;
                            }
                            setTokenValue(ownTokenValue);
                        }
                    }
                }
                //behind another head
                if(inFlags[surfaceParent]->ownTokenValue ==inFlags[surfaceParent]->tokenValue && outFlags[0].changeToken)
                {
                    qDebug()<<"start change off";
                    for(int label = 0; label<10;label++)
                    {
                        outFlags[label].changeToken =  false;
                    }
                }
                //delta off when forward has received, off wave has returned;then start vectors for matching
                if(inFlags[surfaceParent]->tokenValue == ownTokenValue && !inFlags[surfaceParent]->changeToken && outFlags[0].changeToken)
                {
                    for(int label = 0; label<10;label++)
                    {
                        outFlags[label].changeToken =  false;
                        outFlags[label].handleVectors = true;
                    }
                    qDebug()<<"start vectors";
                    setTokenCoordinates(followDir);
                }
                if(outFlags[0].handleVectors)
                {
                    processTokenCoordinates(surfaceParent,surfaceFollower);

                    if(  !inFlags[surfaceParent]->handleVectors && inFlags[surfaceParent]->tokenCurrentDir!=-1
                         && (outFlags[0].xT == -3 ||outFlags[0].xT == 0)
                         && (outFlags[0].yT == -3 ||outFlags[0].yT == 0)
                         && !outFlags[0].xTAccept && !outFlags[0].yTAccept)
                    {
                        qDebug()<<"have leader?";
                        headMarkColor = 0xFFFF00;

                    }
                }
            }
            //non-head
            else if((outFlags[0].tokenValue != outFlags[0].ownTokenValue) )
            {

                //delta passed and is different- prevents secondary forward wave
                if(!outFlags[0].changeToken && inFlags[surfaceFollower]->changeToken
                        && inFlags[surfaceFollower]->tokenValue!=outFlags[0].tokenValue)//because were same before, now explicitely changing
                {
                    for(int label = 0; label<10;label++)
                    {
                        outFlags[label].changeToken =  true;
                        outFlags[label].tokenValue = inFlags[surfaceFollower]->tokenValue;
                    }
                    if(outFlags[0].tokenValue == outFlags[0].ownTokenValue)//accidently became "head", undo that:
                    {
                        qDebug()<<"non head changed own";
                        ownTokenValue = outFlags[0].ownTokenValue;//to be safe
                        int prevOwn = ownTokenValue;
                        while(prevOwn == ownTokenValue)
                        {
                            ownTokenValue = rand() % 8;
                        }

                        for(int label = 0; label<10;label++)
                        {
                            outFlags[label].ownTokenValue= ownTokenValue;
                        }

                    }
                }
                //non-head beat by follower
                else if(!outFlags[0].changeToken && inFlags[surfaceFollower]->tokenValue>outFlags[0].tokenValue)
                {
                    qDebug()<<"nonhead beat";
                    setTokenValue(inFlags[surfaceFollower]->tokenValue);
                    clearVectorToken();

                }

                //delta on, check about off
                else if(outFlags[0].changeToken)
                {
                    //behind head; start change off
                    if(inFlags[surfaceParent]->ownTokenValue ==inFlags[surfaceParent]->tokenValue )
                    {
                        qDebug()<<"start change off";
                        for(int label = 0; label<10;label++)
                        {

                            outFlags[label].changeToken =  false;
                        }
                    }
                    //behind someone with same value and change off- so continue back wave
                    else if(inFlags[surfaceParent]->changeToken == false && inFlags[surfaceParent]->tokenValue == outFlags[0].tokenValue)
                    {
                        qDebug()<<"continue change off";
                        for(int label = 0; label<10;label++)
                        {
                            outFlags[label].changeToken =  false;
                        }
                    }
                }
                //handle tokens passed
                else  if(inFlags[surfaceFollower]->handleVectors && !outFlags[0].handleVectors && outFlags[0].tokenCurrentDir == -1)
                {
                    for(int label = 0; label<10;label++)
                    {
                        outFlags[label].handleVectors =  true;
                    }
                    setTokenCoordinates(surfaceFollower);
                }
                if (outFlags[0].handleVectors)
                {
                    //check if off
                    processTokenCoordinates(surfaceParent,surfaceFollower);

                    //if cleared, see if behind head or someone with handleV off
                    if( (outFlags[0].xT == -3 ||outFlags[0].xT == 0)
                            && (outFlags[0].yT == -3 ||outFlags[0].yT == 0)
                            && !outFlags[0].xTAccept && !outFlags[0].yTAccept)
                    {
                        if(inFlags[surfaceParent]->ownTokenValue == inFlags[surfaceParent]->tokenValue ||
                                !inFlags[surfaceParent]->handleVectors)
                        {
                            for(int label = 0; label<10;label++)
                            {
                                outFlags[label].handleVectors =  false;
                            }
                        }
                    }
                }


            }

            /*   //delta has been unset, but keep processing
            if(!outFlags[0].changeToken && outFlags[0].tokenCurrentDir != -1 && !outFlags[0].handleVectors)
            {
                processTokenCoordinates(surfaceParent,surfaceFollower);
            }*/
        }
    }

    if(phase ==Phase::retiredLeader) clearHeldToken();

    //debugs...
    qDebug()<<"id: "<<ownTokenValue<<" held: "<<outFlags[0].tokenValue<<"token  dir: "<<outFlags[0].tokenCurrentDir<<" changeT: "<<outFlags[0].changeToken<<" handleV: "<<outFlags[0].handleVectors;
    if(surfaceFollower!=-1 && surfaceParent!=-1 && inFlags[surfaceParent]!=nullptr && inFlags[surfaceFollower]!=nullptr)
    {
        qDebug()<<"in front of: "<<inFlags[surfaceFollower]->ownTokenValue<<" behind: "<<inFlags[surfaceParent]->ownTokenValue;
    }


}

void LeaderElectionDemo::clearHeldToken()
{
    for(int label = 0; label<10; label++)
    {
        outFlags[label].tokenValue = -1;
        outFlags[label].tokenD1 = 0;
        outFlags[label].tokenD2 = 0;
        outFlags[label].tokenD3 = 0;
        outFlags[label].tokenCurrentDir = -1;
        outFlags[label].subPhase = -1;

    }
}
void LeaderElectionDemo::clearVectorToken()
{
    for(int label = 0; label<10; label++)
    {
        outFlags[label].xT = -3;
        outFlags[label].yT = -3;
        outFlags[label].xTAccept = false;
        outFlags[label].yTAccept = false;
        outFlags[label].tokenCurrentDir=-1;
        outFlags[label].changeToken = false;
        outFlags[label].handleVectors = false;
    }
}

//sets up the initial tokenCoordinate vectors for matching
void LeaderElectionDemo::setTokenCoordinates(int recDir)
{
    int recOppositeDir = (recDir+3)%6;
    int offset =headMarkDir - recOppositeDir;
    if(offset == 5) offset = -1;
    if(offset == -5) offset = 1;
    int tokenCurrentDir = inFlags[recDir]->tokenCurrentDir;
    if(tokenCurrentDir == -1) //enitialized- so this is axis #1, start counting here
    {
        tokenCurrentDir = 0;
    }
    else
    {
        tokenCurrentDir = (tokenCurrentDir+offset+6)%6;

    }
    for(int label =0; label<10;label++)
    {
        outFlags[label].tokenCurrentDir = tokenCurrentDir;

    }


    int xTtemp = -3;
    int yTtemp = -3;
    switch(tokenCurrentDir)
    {
    case 0:
        xTtemp = 2;
        yTtemp = 0;
        break;
    case 1:
        xTtemp = 1;
        yTtemp = 1;
        break;
    case 2:
        xTtemp = -1;
        yTtemp = 1;
        break;
    case 3:
        xTtemp = -2;
        yTtemp = 0;
        break;
    case 4:
        xTtemp = -1;
        yTtemp = -1;
        break;
    case 5:
        xTtemp = 1;
        yTtemp = -1;
        break;
    }
    for(int label =0; label<10;label++)
    {

        outFlags[label].xT = xTtemp;
        outFlags[label].yT = yTtemp;
        outFlags[label].xTAccept = false;
        outFlags[label].yTAccept = false;
    }
}
//attempts to compress some vector component tokens toward the goal 0,0/blank,blank
void LeaderElectionDemo::processTokenCoordinates(int parent, int follower)
{
    processXTokenCoordinates(parent,follower);
    processYTokenCoordinates(parent,follower);
}
void LeaderElectionDemo::processXTokenCoordinates(int parent, int follower)
{
    if(parent==-1 || follower == -1 || inFlags[parent]==nullptr || inFlags[follower]==nullptr)
        return;

    qDebug()<<"handling xtokens: "<<ownTokenValue<<"  "<<outFlags[0].xT<<" "<<outFlags[0].yT;
    //xTokens
    if(ownTokenValue == outFlags[0].tokenValue)//segment leader/head can clear once token taken, but otherwise they must stop here
    {
        if(outFlags[0].xT>-3 && inFlags[parent]->xTAccept)
        {
            for(int label = 0;label<10;label++)
            {
                outFlags[label].xT = -3;
            }
        }
    }
    else
    {
        if(!outFlags[0].xTAccept && !inFlags[parent]->xTAccept  && !inFlags[follower]->xTAccept)
        {
            qDebug()<<"b1";

            if (outFlags[0].xT>-3 && inFlags[follower]->xT>-3)//-3 is "blank"
            {
                int testSum = outFlags[0].xT+ inFlags[follower]->xT;
                if(testSum>-3 && testSum<3)//manageable range for combining 2 tokens
                {
                    for(int label = 0;label<10;label++)
                    {
                        outFlags[label].xT = testSum;
                        outFlags[label].xTAccept = true;
                    }
                }

            }
            else if(outFlags[0].xT==-3&& inFlags[follower]->xT>-3)
            {
                for(int label = 0;label<10;label++)
                {
                    outFlags[label].xT = inFlags[follower]->xT;
                    outFlags[label].xTAccept = true;
                }
            }
        }
        else if(outFlags[0].xTAccept && inFlags[follower]->xT == -3)
        {
            for(int label = 0;label<10;label++)
            {
                outFlags[label].xTAccept = false;
            }

        }

        else if(outFlags[0].xT>-3 && inFlags[parent]->xTAccept)
        {
            for(int label = 0;label<10;label++)
            {
                outFlags[label].xT = -3;
            }
        }

    }
    qDebug()<<"xT: "<<outFlags[0].xT<<" accept? "<<outFlags[0].xTAccept;
    //ytokens

}
void LeaderElectionDemo::processYTokenCoordinates(int parent, int follower)
{
    if(parent==-1 || follower == -1 || inFlags[parent]==nullptr || inFlags[follower]==nullptr)
        return;

    qDebug()<<"handling ytokens: "<<ownTokenValue<<"  "<<outFlags[0].xT<<" "<<outFlags[0].yT;
    //xTokens
    if(ownTokenValue == outFlags[0].tokenValue)//segment leader/head can clear once token taken, but otherwise they must stop here
    {
        if(outFlags[0].yT>-3 && inFlags[parent]->yTAccept)
        {
            for(int label = 0;label<10;label++)
            {
                outFlags[label].yT = -3;
            }
        }
    }
    else
    {
        if(!outFlags[0].yTAccept && !inFlags[parent]->yTAccept  && !inFlags[follower]->yTAccept)
        {
            qDebug()<<"b1";

            if (outFlags[0].yT>-3 && inFlags[follower]->yT>-3)//-3 is "blank"
            {
                int testSum = outFlags[0].yT+ inFlags[follower]->yT;
                if(testSum>-3 && testSum<3)//manageable range for combining 2 tokens
                {
                    for(int label = 0;label<10;label++)
                    {
                        outFlags[label].yT = testSum;
                        outFlags[label].yTAccept = true;
                    }
                }

            }
            else if(outFlags[0].yT==-3&& inFlags[follower]->yT>-3)
            {
                for(int label = 0;label<10;label++)
                {
                    outFlags[label].yT = inFlags[follower]->yT;
                    outFlags[label].yTAccept = true;
                }
            }
        }
        else if(outFlags[0].yTAccept && inFlags[follower]->yT == -3)
        {
            for(int label = 0;label<10;label++)
            {
                outFlags[label].yTAccept = false;
            }

        }

        else if(outFlags[0].yT>-3 && inFlags[parent]->yTAccept)
        {
            for(int label = 0;label<10;label++)
            {
                outFlags[label].yT = -3;
            }
        }

    }
    qDebug()<<"yT: "<<outFlags[0].yT<<" accept? "<<outFlags[0].yTAccept;
    //ytokens

}

void LeaderElectionDemo::setTokenValue(int value)
{
    for(int label=0; label<10; label++)
        outFlags[label].tokenValue = value;
}



}
