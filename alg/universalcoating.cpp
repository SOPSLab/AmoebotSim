
#include <algorithm>
#include <cmath>
#include <deque>
#include <set>

#include "alg/universalcoating.h"
#include "sim/particle.h"
#include "sim/system.h"
#include <QDebug>


namespace UniversalCoating
{

UniversalCoatingFlag::UniversalCoatingFlag()
    : followIndicator(false),
      Lnumber(-1),
      NumFinishedNeighbors(0),
      leadComplaint(false),
      seedBound(false),
      block(false),
      tokenValue(-1),
      tokenD1(0),
      tokenD2(0),
      tokenD3(0),
      tokenCurrentDir(-1),
      isSendingToken(false),
      ownTokenValue(-1),
      buildBorder(false),
      acceptPositionTokens(false),
      electionRole(ElectionRole::Pipeline),
      electionSubphase(ElectionSubphase::Wait)


{
    int typenum = 0;
    for(auto token = tokens.begin(); token != tokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        qDebug()<<(int)token->type<<" value "<<token->value;
        ++typenum;
    }

}

UniversalCoatingFlag::UniversalCoatingFlag(const UniversalCoatingFlag& other)
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
      tokenD1(other.tokenD1),
      tokenD2(other.tokenD2),
      tokenD3(other.tokenD3),
      tokenCurrentDir(other.tokenCurrentDir),
      isSendingToken(other.isSendingToken),
      ownTokenValue(other.ownTokenValue),
      buildBorder(other.buildBorder),
      tokens(other.tokens),
      acceptPositionTokens(other.acceptPositionTokens),
      electionRole(other.electionRole),
      electionSubphase(other.electionSubphase)

{
}

UniversalCoating::UniversalCoating(const Phase _phase)
{
    setPhase(_phase);
    Lnumber = -1;
    downDir = -1;
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
    ownTokenValue = -1;
    hasLost = false;
    superLeader= false;
    borderPasses = 0;
    id = -1;
    electionRole = ElectionRole::Pipeline;
    electionSubphase = ElectionSubphase::Wait;
    waitingForTransferAck = false;
    gotAnnounceBeforeAck = false;
    generateVectorDir = -1;
    createdLead = false;
    sawUnmatchedToken = false;
    comparingSegment= false;
    absorbedActiveToken = false;
    isCoveredCandidate = false;
    gotAnnounceInCompare= false;


}

UniversalCoating::UniversalCoating(const UniversalCoating& other)
    : AlgorithmWithFlags(other),
      phase(other.phase),
      followDir(other.followDir),
      Lnumber(other.Lnumber),
      downDir(other.downDir),
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
      id(other.id),
      electionRole(other.electionRole),
      electionSubphase(other.electionSubphase),
      waitingForTransferAck(other.waitingForTransferAck),
      gotAnnounceBeforeAck(other.gotAnnounceBeforeAck),
      generateVectorDir(other.generateVectorDir),
      createdLead(other.createdLead),
      sawUnmatchedToken(other.sawUnmatchedToken),
      comparingSegment(other.comparingSegment),
      absorbedActiveToken(other.absorbedActiveToken),
      isCoveredCandidate(other.isCoveredCandidate),
      gotAnnounceInCompare(other.gotAnnounceInCompare)

{
}

UniversalCoating::~UniversalCoating()
{
}

std::shared_ptr<System> UniversalCoating::instance(const int numStaticParticles, const int numParticles, const float holeProb, const bool leftBorder, const bool rightBorder)
{
    std::shared_ptr<System> system = std::make_shared<System>();
    std::deque<Node> orderedSurface;
    std::set<Node> occupied;
    Node pos;
    Node Start;
    Node End;
    int lastOffset = 0;

    bool first = !leftBorder && !rightBorder;
    while(system->size() < numStaticParticles) {
        if(first)
        {
            system->insertParticle(Particle(std::make_shared<UniversalCoating>(Phase::Static), randDir(), pos));
            first = false;
        }
        else
        {
            system->insertParticle(Particle(std::make_shared<UniversalCoating>(Phase::Static), randDir(), pos));
        }
        occupied.insert(pos);
        orderedSurface.push_back(pos);
        int offset;
        if(lastOffset == 4) {
            offset = randInt(3, 5);
        } else {
            offset = randInt(2, 5);
        }
        lastOffset = offset;
        End=pos;
        pos = pos.nodeInDir(offset);
    }

    //arbitrary borders
    lastOffset = 0;
    int counter=0;
    int yMax= numParticles;
    Node pos1= Node(Start.x + 1 ,Start.y);//start of the left border
    Node pos2= Node((End.x)-1 ,End.y);//start of the right border
    if(leftBorder)
    {
        //while(counter < yMax) {
        system->insertParticle( Particle(std::make_shared<UniversalCoating>(Phase::StaticBorder), randDir(), pos1));

        occupied.insert(pos1);
        orderedSurface.push_back(pos1);
        int offset;
        if(lastOffset == 3) {
            offset = randInt(1, 4);
        } else {
            offset = randInt(1, 3);
        }
        lastOffset = offset;
        pos1 = pos1.nodeInDir(offset);
        counter++;
        // }
    }
    if(rightBorder)
    {
        counter =0;
        //  while(counter < yMax) {
        system->insertParticle( Particle(std::make_shared<UniversalCoating>(Phase::StaticBorder), randDir(), pos2));
        occupied.insert(pos2);
        orderedSurface.push_back(pos2);
        int offset;
        if(lastOffset == 2) {
            offset = randInt(1, 3);
        } else {
            offset = randInt(1, 4);
        }
        lastOffset = offset;
        pos2 = pos2.nodeInDir(offset);
        counter++;
        //   }

    }
    //arbitrary borders

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
    int idCounter = 0;

    while(numNonStaticParticles < numParticles) {
        if(candidates.empty()) {
            return system;
        }

        std::set<Node> nextCandidates;
        for(auto it = candidates.begin(); it != candidates.end() && numNonStaticParticles < numParticles; ++it) {
            if(randBool(1.0f - holeProb)) {
                std::shared_ptr<UniversalCoating> newParticle= std::make_shared<UniversalCoating>(Phase::Inactive);
                newParticle->id = idCounter;
                system->insertParticle(Particle(newParticle, randDir(), *it));
                // system->insertParticle(Particle(std::make_shared<UniversalCoating>(Phase::Inactive), randDir(), *it));
                numNonStaticParticles++;

                for(int dir = 1; dir <= 2; dir++) {
                    const Node node = it->nodeInDir(dir);
                    if(occupied.find(node) == occupied.end()) {
                        nextCandidates.insert(node);
                        occupied.insert(node);

                        if(node.y > yMax) {
                            yMax = node.y;
                        }
                    }
                }
            }
            idCounter++;

        }
        nextCandidates.swap(candidates);
    }

    return system;
}

Movement UniversalCoating::subExecute()
{

    auto surfaceFollower = firstNeighborInPhase(Phase::Static);
    int   surfaceParent = headMarkDir;
    if(surfaceFollower!=-1)
    {
        while(neighborIsInPhase(surfaceFollower,Phase::Static))
            surfaceFollower = (surfaceFollower+1)%6;
    }
    Movement movement = subExecute();

    /*      if(movement.type == MovementType::Expand)//whether expanding as a handover or into empty space, tail stays where head currently is
        {
            qDebug()<<"expand";
            movePositionTokens(true);
            copyParentPositionTokens(headMarkDir);//head points at parent
          outFlags[headMarkDir].acceptPositionTokens = true;
        }
        else if(movement.type == MovementType::Contract)
        {
            qDebug()<<"contract";
            outFlags[headMarkDir].acceptPositionTokens = false;

          //maybe keep both??
        }
        else  if(movement.type == MovementType::HandoverContract)
        {
            qDebug()<<"handover";
            outFlags[headMarkDir].acceptPositionTokens = false;

            if(surfaceFollower > -1 && inFlags[surfaceFollower]!=nullptr && inFlags[surfaceFollower]->acceptPositionTokens)
                clearPositionTokens(false);//clear tail
            else
                return Movement(MovementType::Empty);

        }
        else  if(movement.type == MovementType::Idle)
        {

        }
*/
    return movement;
}

Movement UniversalCoating::execute()
{



    for(int i =0; i<10;i++)
        outFlags[i].id = id;

    updateNeighborStages();

    if(phase == Phase::Lead || phase == Phase::retiredLeader)
        unsetFollowIndicator();

    if(hasNeighborInPhase(Phase::Static))
    {
        if(phase!=Phase::Inactive  && isContracted())
            handlePositionElection();

    }

    if(isExpanded()) {
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
            if(inFlags[label] != nullptr && !neighborIsInPhase(label,Phase::Static) && !neighborIsInPhase(label,Phase::StaticBorder)) {
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

            }
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        } else {
            if(phase==Phase::Hold && isSuperLeader())
            {
                setPhase(Phase::Normal);
            }
            return Movement(MovementType::Contract, tailContractionLabel());
        }
    } else
        //contracted
    {

        if(phase!=Phase::Inactive && phase!=Phase::Static && phase!=Phase::Border && phase!=Phase::StaticBorder)
        {
            //check if on static border
            if(hasNeighborInPhase(Phase::StaticBorder) && !hasNeighborInPhase(Phase::Static))
            {
                headMarkDir = firstNeighborInPhase(Phase::StaticBorder);
                setPhase(Phase::Border);
                int borderBuildDir = (headMarkDir+3)%6;
                while(neighborIsInPhase(borderBuildDir,Phase::Static) || neighborIsInPhase(borderBuildDir,Phase::StaticBorder))
                    borderBuildDir = (borderBuildDir+1)%6;
                outFlags[borderBuildDir].buildBorder= true;
                return Movement(MovementType::Idle);
            }
            else if(phase==Phase::retiredLeader || phase == Phase::Lead)
                // || phase == Phase::Follow || phase == Phase::Wait
                //||phase == Phase::Hold || phase == Phase::Send) //check if on regular border
            {
                for(int label= 0; label<10;label++)
                {
                    if(inFlags[label]!=nullptr && inFlags[label]->buildBorder)
                    {

                        headMarkDir = label;
                        setPhase(Phase::Border);
                        int borderBuildDir = (headMarkDir+3)%6;
                        while(neighborIsInPhase(borderBuildDir,Phase::Static) || neighborIsInPhase(borderBuildDir,Phase::StaticBorder))
                            borderBuildDir = (borderBuildDir+1)%6;
                        outFlags[borderBuildDir].buildBorder= true;
                        qDebug()<<"trying to border?";
                        setPhase(Phase::Border);
                        return Movement(MovementType::Idle);
                    }
                }
            }
            setBlock();
            if(hasNeighborInPhase(Phase::Static) || hasNeighborInPhase(Phase::StaticBorder) )
            {
                //
                //handleElectionTokens();
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
            if(hasNeighborInPhase(Phase::Static) || hasNeighborInPhase(Phase::StaticBorder)) {
                setPhase(Phase::Normal);
                startedOffSurface = false;

                downDir= getDownDir();
                if(downDir == -1)
                    return Movement(MovementType::Idle);
                else downDir=labelToDir(downDir);

                Lnumber = getLnumber();
                setLayerNumber(Lnumber);

                setElectionRole(ElectionRole::Candidate);


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

        else if(phase!=Phase::Static && phase!=Phase::Border && phase!=Phase::StaticBorder)
        {
            //layering block
            if(hasNeighborInPhase(Phase::Static) || hasNeighborInPhase(Phase::StaticBorder) )
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
                if(hasNeighborInPhase(Phase::Static) || hasNeighborInPhase(Phase::StaticBorder))
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

                else if(hasNeighborInPhase(Phase::Static)|| hasNeighborInPhase(Phase::StaticBorder))
                {
                    setPhase(Phase::Normal);
                }

            }
            else if(phase == Phase:: Hold)
            {
                if((hasNeighborInPhase(Phase::Static)|| hasNeighborInPhase(Phase::StaticBorder))
                        &&!(parentActivated()||neighborIsInPhase(headMarkDir,Phase::Inactive)))
                {
                    int moveDir = getMoveDir();
                    setContractDir(moveDir);
                    headMarkDir = moveDir;
                    followDir = headMarkDir;
                    auto temp = dirToHeadLabelAfterExpansion(followDir, moveDir);
                    setFollowIndicatorLabel(temp);

                    if(!(inFlags[moveDir]!=nullptr && (inFlags[moveDir]->isContracted())) )
                    {
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

                return Movement(MovementType::Expand, moveDir);

            }
            else if(phase == Phase::Follow) {
                if(inFlags[followDir]==nullptr)//with bordering, this loss of leader can happen
                {
                    setPhase(Phase::Normal);
                    qDebug()<<"used leads2";

                    return Movement(MovementType::Idle);
                }
                //Q_ASSERT(inFlags[followDir] != nullptr);
                if(hasNeighborInPhase(Phase::Static) || hasNeighborInPhase(Phase::StaticBorder)) {
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
                /* for(int label= 0; label<10;label++)
                {
                    if(inFlags[label]!=nullptr && inFlags[label]->buildBorder)
                    {
                        qDebug()<<"retiredLeader gets border";
                        headMarkDir = label;
                        setPhase(Phase::Border);
                        int borderBuildDir = (headMarkDir+3)%6;
                        while(neighborIsInPhase(borderBuildDir,Phase::Static) || neighborIsInPhase(borderBuildDir,Phase::StaticBorder))
                            borderBuildDir = (borderBuildDir+1)%6;
                        outFlags[borderBuildDir].buildBorder= true;
                        qDebug()<<"trying to border?";
                        return Movement(MovementType::Idle);
                    }
                }*/
                getLeftDir();
                Q_ASSERT(leftDir >=0 && leftDir<=5);
                NumFinishedNeighbors = CountFinishedSides(leftDir, rightDir);
                setNumFinishedNeighbors(NumFinishedNeighbors);
                Q_ASSERT(NumFinishedNeighbors>=0 && NumFinishedNeighbors<=2);
                /* if(NumFinishedNeighbors==2)
                {
                    if(reachedSeedBound)
                    {
                        int seedBoundFlagDir= (downDir+3)%6;
                        int tries = 0;
                        while(neighborIsInPhase(seedBoundFlagDir,Phase::Static) || neighborIsInPhase(seedBoundFlagDir,Phase::StaticBorder)
                              || ( neighborIsInPhase(seedBoundFlagDir,Phase::retiredLeader) && tries<10 ) )
                        {
                            seedBoundFlagDir = (seedBoundFlagDir+1)%6;
                            tries++;
                        }
                        outFlags[seedBoundFlagDir].seedBound = true;
                    }
                }*/
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
                        return Movement(MovementType::Expand, expansionDir);
                    }
                }



                return Movement(MovementType::Idle);
            }
        }
    }

    return Movement(MovementType::Empty);
}

std::shared_ptr<Algorithm> UniversalCoating::blank() const
{
    return std::make_shared<UniversalCoating>(Phase::Inactive);
}

std::shared_ptr<Algorithm> UniversalCoating::clone()
{
    return std::make_shared<UniversalCoating>(*this);
}

bool UniversalCoating::isDeterministic() const
{
    return true;
}

void UniversalCoating::setPhase(const Phase _phase)
{
    phase = _phase;

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
        //  if(NumFinishedNeighbors ==2)
        // {
        headMarkColor = 0x00ff00;
        tailMarkColor = 0x00ff00;
        //}
    }
    else if(phase == Phase::Seed)
    {
        headMarkColor = 0xFF69B4;
        tailMarkColor = 0xFF69B4;
    }

    else if(phase == Phase::Inactive) {
        headMarkColor = -1;
        tailMarkColor = -1;
    } else  if(phase == Phase::Border || phase == Phase::StaticBorder)
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

    for(int label = 0; label < 10; label++) {
        outFlags[label].phase = phase;
    }
    /*if(phase!=Phase::Static && hasNeighborInPhase(Phase::Static))
    {
        headMarkColor = 0xFF0000;
        tailMarkColor = 0xFF0000;
    }

    else if(phase!=Phase::Static)
    {
        headMarkColor = 0x0000FF;
        tailMarkColor = 0x0000FF;
    }
*/
    //new colors block...
    if(phase == Phase::Normal || phase == Phase::Wait )
    {
        headMarkColor = 0x0000FF;
        tailMarkColor = 0x0000FF;
    }
    else if(phase == Phase::Hold || phase ==Phase::Send)
    {
        headMarkColor = 0xFF69B4;
        tailMarkColor = 0xFF69B4;
    }
    else if(phase ==Phase::Lead)
    {
        headMarkColor = 0xFF0000;
        tailMarkColor = 0xFF0000;
    }
}

bool UniversalCoating::neighborIsInPhase(const int label, const Phase _phase) const
{
    Q_ASSERT(0 <= label && label <= 9);
    return (inFlags[label] != nullptr && inFlags[label]->phase == _phase);
}

bool UniversalCoating::neighborIsInPhaseandLayer(const int label, const Phase _phase, const int L) const
{
    Q_ASSERT(isContracted());

    return (inFlags[label] != nullptr && inFlags[label]->phase == _phase && inFlags[label]->Lnumber == L);
}

int UniversalCoating::firstNeighborInPhase(const Phase _phase) const
{

    for(int label = 0; label < 10; label++) {
        if(neighborIsInPhase(label, _phase)) {
            return label;
        }
    }
    return -1;
}

int UniversalCoating::firstNeighborInPhaseandLayer(const Phase _phase, const int L) const
{
    for(int label = 0; label < 10; label++) {
        if(neighborIsInPhaseandLayer(label, _phase, L)){
            return label;
        }
    }
    return -1;
}
bool UniversalCoating::hasNeighborInPhase(const Phase _phase) const
{
    return (firstNeighborInPhase(_phase) != -1);
}

void UniversalCoating::setContractDir(const int contractDir)
{
    for(int label = 0; label < 10; label++) {
        outFlags[label].contractDir = contractDir;
    }
}

int UniversalCoating::updatedFollowDir() const
{
    int contractDir = inFlags[followDir]->contractDir;
    int offset = (followDir - inFlags[followDir]->dir + 9) % 6;
    int tempFollowDir = (contractDir + offset) % 6;
    Q_ASSERT(0 <= tempFollowDir && tempFollowDir <= 5);
    return tempFollowDir;
}

void UniversalCoating::unsetFollowIndicator()
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = false;
    }
}

void UniversalCoating::setFollowIndicatorLabel(const int label)
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = (i == label);
    }
}

bool UniversalCoating::tailReceivesFollowIndicator() const
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

int UniversalCoating::getMoveDir() const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(Lnumber!=-1);
    int label= -1;
    if(Lnumber %2 == 0)
    {
        label = firstNeighborInPhase(Phase::Static);
        if(label==-1)
            label = firstNeighborInPhase(Phase::StaticBorder);
        if(label != -1)
        {
            label = (label+5)%6;
            while (neighborIsInPhase(label, Phase::Static) || neighborIsInPhase(label,Phase::StaticBorder)){
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
        if(label==-1)
            label = firstNeighborInPhase(Phase::StaticBorder);

        if(label != -1)
        {
            label = (label+1)%6;
            while (neighborIsInPhase(label, Phase::Static)  || neighborIsInPhase(label,Phase::StaticBorder)){
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

int UniversalCoating::getDownDir() const
{
    Q_ASSERT(isContracted());
    //Q_ASSERT(phase==Phase::Lead);

    int label= -1;
    if(hasNeighborInPhase(Phase::Static)) {
        label = firstNeighborInPhase(Phase::Static);

    }
    else if(hasNeighborInPhase(Phase::StaticBorder))
    {
        label = firstNeighborInPhase(Phase::StaticBorder);
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

int UniversalCoating::getLnumber() const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(downDir != -1);
    Q_ASSERT(inFlags[downDir] != nullptr);
    if(inFlags[downDir] != nullptr)
    {
        if(inFlags[downDir]->phase == Phase::Static || inFlags[downDir]->phase == Phase::StaticBorder)
            return 0;
        else
            return (inFlags[downDir]->Lnumber +1)%2;
    }
    //Method has to return an integer in every case. Please check if 0 is a suitable value in this case. [Stanislaw Eppinger]
    return 0;
}

void UniversalCoating::getLeftDir()
{
    Q_ASSERT(isContracted());
    Q_ASSERT(phase == Phase::retiredLeader);
    Q_ASSERT(downDir != -1);

    int label= downDir;
    int leftside= -1, rightside= -1;
    if(neighborIsInPhase(label, Phase::Static) || neighborIsInPhase(label, Phase::StaticBorder))
    {
        label = (label+5)%6;
        while (neighborIsInPhase(label, Phase::Static)|| neighborIsInPhase(label, Phase::StaticBorder)){
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
    if(neighborIsInPhase(label, Phase::Static)|| neighborIsInPhase(label, Phase::StaticBorder))
    {
        label = (label+1)%6;
        while (neighborIsInPhase(label, Phase::Static) || neighborIsInPhase(label, Phase::StaticBorder)){
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

int UniversalCoating::countGreenNeighbors(const Phase _phase, const int L) const
{
    int count=0;
    for(int dir = 0; dir < 10; dir++) {
        if(neighborIsInPhaseandLayer(dir, _phase, L) ) {
            count++;
        }
    }
    return count;
}
void UniversalCoating::setLayerNumber(const int _Lnumber){
    Q_ASSERT(_Lnumber==0 || _Lnumber==1);
    for(int label = 0; label < 10; label++){
        outFlags[label].Lnumber= _Lnumber;
    }
}

int UniversalCoating::CountFinishedSides(const int _leftDir, const int _rightDir) const
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

int UniversalCoating::countRetiredareFinished(const int _Lnumber) const
{
    int countFinished =0;
    for(int dir = 0; dir < 10; dir++) {
        if(neighborIsInPhaseandLayer(dir, Phase::retiredLeader, _Lnumber) && inFlags[dir]->NumFinishedNeighbors == 2) {
            countFinished++; //number of retired leaders p has with layer number =0

        }
    }

    return countFinished;
}

void UniversalCoating::setNumFinishedNeighbors(const int _NumFinishedNeighbors)
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].NumFinishedNeighbors = _NumFinishedNeighbors;
    }
}
bool UniversalCoating::inFrontOfLeadC() const
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
void UniversalCoating::setLeadComplaint(bool value)
{
    for(int label = 0; label<10;label++)
    {
        outFlags[label].leadComplaint = value;
    }

}
void UniversalCoating::setBlock()
{

    bool block = false;

    if(hasNeighborInPhase(Phase::Static)|| hasNeighborInPhase(Phase::StaticBorder))
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
        if(label==-1)
            label = firstNeighborInPhase(Phase::StaticBorder);
        int behindLabel = (label+1)%6;
        int frontLabel = label;
        while(neighborIsInPhase(frontLabel,Phase::Static)|| neighborIsInPhase(frontLabel,Phase::StaticBorder))
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
void UniversalCoating::expandedSetBlock()
{


}
bool UniversalCoating::isSuperLeader()
{

    int headNeighbors = 0;
    int tailNeighbors = 0;
    auto label = firstNeighborInPhase(Phase::Static);
    if(label==-1)
    {
        label = firstNeighborInPhase(Phase::StaticBorder);
    }
    if(label != -1)
    {
        label = (label+9)%10;
        while (neighborIsInPhase(label, Phase::Static) || neighborIsInPhase(label, Phase::StaticBorder) ){
            label = (label+9)%10;
        }
        if(inFlags[label]!=nullptr)
            headNeighbors++;
    }
    label = firstNeighborInPhase(Phase::Static);
    if(label==-1)
    {
        label = firstNeighborInPhase(Phase::StaticBorder);

    }
    if(label != -1)
    {
        label = (label+1)%10;
        if(inFlags[label]!=nullptr)
            tailNeighbors++;

    }
    if((hasNeighborInPhase(Phase::Static)|| (hasNeighborInPhase(Phase::StaticBorder))) && headNeighbors==0)// && tailNeighbors == 1)
        return true;
    return false;
}
void UniversalCoating::updateParentStage()
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
void UniversalCoating::updateChildStage()
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
void UniversalCoating::updateNeighborStages()
{
    if(phase ==Phase::Send || phase==Phase::Hold)
        updateParentStage();
    else
        parentStage = -1;


    if(phase!=Phase::Hold)
        childStage = -1;
}

bool UniversalCoating::parentActivated()
{
    if(neighborIsInPhase(headMarkDir,Phase::Hold) || neighborIsInPhase(headMarkDir,Phase::Send)
            ||neighborIsInPhase(headMarkDir,Phase::Wait) || neighborIsInPhase(headMarkDir,Phase::Normal))
        return true;

    return false;
}

void UniversalCoating::handlePositionElection()
{
    qDebug()<<"id: "<<id;
    qDebug()<<"role: "<<(int)electionRole<<" subphase: "<<(int)electionSubphase;


    //step 0: setup
    auto surfaceFollower = firstNeighborInPhase(Phase::Static);
    int   surfaceParent = headMarkDir;
    if(surfaceFollower!=-1)
    {
        while(neighborIsInPhase(surfaceFollower,Phase::Static))
        {
            surfaceFollower = (surfaceFollower+1)%6;
        }

    }
    if( surfaceFollower<0 || surfaceParent<0 ||
            neighborIsInPhase(surfaceParent,Phase::Inactive) || neighborIsInPhase(surfaceFollower,Phase::Inactive ) ||
            inFlags[surfaceFollower] ==nullptr || inFlags[surfaceParent]==nullptr)
    {
        return;
    }
    qDebug()<<"parent: "<<inFlags[surfaceParent]->id<<" follow: "<<inFlags[surfaceFollower]->id;

    if(isExpanded() || inFlags[surfaceFollower]->isExpanded() || inFlags[surfaceParent]->isExpanded())
    {
        qDebug()<<"waiting for contracted positioning";
        return;
    }
    else
    {
        tokenCleanup(surfaceParent,surfaceFollower);

        if(electionRole==ElectionRole::Pipeline)
        {
            //pass along subphase info too
            /*  if(electionSubphase == ElectionSubphase::Wait && inFlags[surfaceFollower]->electionSubphase==ElectionSubphase::SegmentComparison)
            {
                setElectionSubphase(ElectionSubphase::SegmentComparison);
            }*/
            if(inFlags[surfaceFollower]->electionSubphase==ElectionSubphase::CoinFlip)
            {
                setElectionSubphase(ElectionSubphase::CoinFlip);
            }
            else if(inFlags[surfaceFollower]->electionSubphase==ElectionSubphase::SolitudeVerification)
            {
                //ensure clean up of coin flipping
                clearToken(TokenType::CandidacyAck,-1);//-1 clears all dirs
                clearToken(TokenType::CandidacyAnnounce,-1);//-1 clears all dirs
                setElectionSubphase(ElectionSubphase::SolitudeVerification);
            }
            if(electionSubphase==ElectionSubphase::CoinFlip)
            {
                nonCandidateCoinFlipping(surfaceParent,surfaceFollower);
            }
           else  if (electionSubphase == ElectionSubphase::SolitudeVerification)
            {
                nonCandidateSolitudeHandling(surfaceParent,surfaceFollower);

            }

        }
        else if(electionRole == ElectionRole::Candidate)
        {
            //for any subphase...

            // if there is an announcement waiting to be received by me and it is safe to create an acknowledgement, consume the announcement
            if(peekAtToken(TokenType::CandidacyAnnounce, surfaceFollower) != -1 && canSendToken(TokenType::CandidacyAck, surfaceFollower)) {
                receiveToken(TokenType::CandidacyAnnounce, surfaceFollower);
                sendToken(TokenType::CandidacyAck, surfaceFollower, 1);
                if(waitingForTransferAck) {
                    gotAnnounceBeforeAck = true;
                } else if(comparingSegment) {
                    gotAnnounceInCompare = true;
                }
            }

            // if there is a solitude lead token waiting to be put into lane 2, put it there if it doesn't pass a vector token
            if(peekAtToken(TokenType::SolitudeLeadL1, surfaceFollower) != -1 && canSendToken(TokenType::SolitudeLeadL2, surfaceFollower) &&
                    canSendToken(TokenType::SolitudeVectorL2, surfaceFollower)) {
                int leadValue = receiveToken(TokenType::SolitudeLeadL1, surfaceFollower).value;
                if(leadValue / 100 == 1) { // if the lead is on lap 1, append this candidate's local id
                    sendToken(TokenType::SolitudeLeadL2, surfaceFollower, (1000 * 1) + leadValue);
                } else { // if the lead is on lap 2, just pass on the value
                    sendToken(TokenType::SolitudeLeadL2, surfaceFollower, leadValue);
                }
            }
            // if there is a vector waiting to be put into lane 2, put it there
            if(peekAtToken(TokenType::SolitudeVectorL1, surfaceFollower) != -1 && canSendToken(TokenType::SolitudeVectorL2, surfaceFollower)) {
                sendToken(TokenType::SolitudeVectorL2, surfaceFollower, receiveToken(TokenType::SolitudeVectorL1, surfaceFollower).value);
            }

            if(electionSubphase == ElectionSubphase::Wait)
            {
                setElectionSubphase(ElectionSubphase::CoinFlip);
            }
            else if(electionSubphase == ElectionSubphase::CoinFlip)
            {
                if(peekAtToken(TokenType::CandidacyAck, surfaceParent) != -1) {
                    // if there is an acknowledgement waiting, consume the acknowledgement and proceed to the next subphase
                    receiveToken(TokenType::CandidacyAck, surfaceParent);
                    setElectionSubphase(ElectionSubphase::SolitudeVerification);
                    if(!gotAnnounceBeforeAck) {
                        setElectionRole(ElectionRole::Demoted);
                    }
                    waitingForTransferAck = false;
                    gotAnnounceBeforeAck = false;
                    return; // completed subphase and thus shouldn't perform any more operations in this round
                } else if(!waitingForTransferAck && randBool()) {
                    // if I am not waiting for an acknowlegdement of my previous announcement and I win the coin flip, announce a transfer of candidacy
                    Q_ASSERT(canSendToken(TokenType::CandidacyAnnounce, surfaceParent) && canSendToken(TokenType::PassiveSegmentClean, surfaceParent)); // there shouldn't be a call to make two announcements
                    sendToken(TokenType::CandidacyAnnounce, surfaceParent, 1);
                    waitingForTransferAck = true;
                }
            }
            else if (electionSubphase == ElectionSubphase::SolitudeVerification)
            {
                //ensure clean up of coin flipping
                clearToken(TokenType::CandidacyAck,-1);//-1 clears all dirs
                clearToken(TokenType::CandidacyAnnounce,-1);//-1 clears all dirs
                // if the agent needs to, generate a lane 1 vector token
                if(generateVectorDir != -1 && canSendToken(TokenType::SolitudeVectorL1, surfaceParent) && canSendToken(TokenType::PassiveSegmentClean, surfaceParent)) {
                    sendToken(TokenType::SolitudeVectorL1, surfaceParent, generateVectorDir);
                    generateVectorDir = -1;
                }

                if(peekAtToken(TokenType::SolitudeVectorL2, surfaceParent) != -1) {
                    // consume all vector tokens that have not been matched, decide that solitude has failed
                    Q_ASSERT(peekAtToken(TokenType::SolitudeVectorL2, surfaceParent) != 0); // matched tokens should have dropped
                    receiveToken(TokenType::SolitudeVectorL2, surfaceParent);
                    sawUnmatchedToken = true;
                    qDebug()<<"saw unmatched";
                } else if(peekAtToken(TokenType::SolitudeLeadL2, surfaceParent) != -1) {
                    qDebug()<<"lead returned";
                    // if the lead token has returned, either put it back in lane 1 (lap 1) or consume it and decide solitude (lap 2)
                    if((peekAtToken(TokenType::SolitudeLeadL2, surfaceParent) % 1000) / 100 == 1) { // lead has just completed lap 1
                        qDebug()<<"lap 1 complete: "<<peekAtToken(TokenType::SolitudeLeadL2, surfaceParent);
                        qDebug()<<"can sends? "<<canSendToken(TokenType::SolitudeLeadL1, surfaceParent)<<" "<<canSendToken(TokenType::SolitudeVectorL1, surfaceParent);
                        if(canSendToken(TokenType::SolitudeLeadL1, surfaceParent) && canSendToken(TokenType::SolitudeVectorL1, surfaceParent))// && canSendToken(TokenType::PassiveSegmentClean, surfaceParent))
                        {
                            int leadValue = receiveToken(TokenType::SolitudeLeadL2, surfaceParent).value;
                            sendToken(TokenType::SolitudeLeadL1, surfaceParent, (leadValue / 1000) * 1000 + 200); // lap 2, orientation no longer matters => 200
                            qDebug()<<"started lap 2";
                        }
                    } else if((peekAtToken(TokenType::SolitudeLeadL2, surfaceParent) % 1000) / 100 == 2) { // lead has just completed lap 2
                        int leadValue = receiveToken(TokenType::SolitudeLeadL2, surfaceParent).value;
                        if(!sawUnmatchedToken && (leadValue / 1000) == 1) { // if it did not consume an unmatched token and it assures it's matching with itself, go to inner/outer test
                            setElectionRole(ElectionRole::SoleCandidate);
                            qDebug()<<"set sole candidate";
                        } else { // if solitude verification failed, then do another coin flip compeititon
                            setElectionSubphase(ElectionSubphase::CoinFlip);
                            qDebug()<<"failed solitude verification";
                        }
                        createdLead = false;
                        sawUnmatchedToken = false;
                        return; // completed subphase and thus shouldn't perform any more operations in this round
                    }
                } else if(!createdLead) {
                    qDebug()<<"begin solitude verification?";
                    // to begin the solitude verification, create a lead token with an orientation to communicate to its segment
                    Q_ASSERT(canSendToken(TokenType::SolitudeLeadL1, surfaceParent) && canSendToken(TokenType::SolitudeVectorL1, surfaceParent) &&
                             canSendToken(TokenType::PassiveSegmentClean, surfaceParent)); // there shouldn't be a call to make two leads
                    sendToken(TokenType::SolitudeLeadL1, surfaceParent, 100 + encodeVector(std::make_pair(1,0))); // lap 1 in direction (1,0)
                    createdLead = true;
                    generateVectorDir = encodeVector(std::make_pair(1,0));
                }
            }


        }
        else if(electionRole == ElectionRole::Demoted)
        {
            //switching subphases:

            //if follower has switched to segment comparison, switch and send token
            if(electionSubphase == ElectionSubphase::Wait && inFlags[surfaceFollower]->electionSubphase==ElectionSubphase::SegmentComparison)
            {
                setElectionSubphase(ElectionSubphase::SegmentComparison);
                //  setToken(TokenType::PassiveSegment,surfaceFollower,1);
            }
            else if(inFlags[surfaceFollower]->electionSubphase==ElectionSubphase::CoinFlip)
            {
                setElectionSubphase(ElectionSubphase::CoinFlip);
            }
            else if(inFlags[surfaceFollower]->electionSubphase==ElectionSubphase::SolitudeVerification)
            {
                //ensure clean up of coin flipping
                clearToken(TokenType::CandidacyAck,-1);//-1 clears all dirs
                clearToken(TokenType::CandidacyAnnounce,-1);//-1 clears all dirs
                setElectionSubphase(ElectionSubphase::SolitudeVerification);
            }

            //subphase-specific action:
            if(electionSubphase == ElectionSubphase::CoinFlip)
            {
                nonCandidateCoinFlipping(surfaceParent,surfaceFollower);

            }
            if (electionSubphase == ElectionSubphase::SolitudeVerification)
            {
                nonCandidateSolitudeHandling(surfaceParent,surfaceFollower);

            }

        }

    }


}
void UniversalCoating::nonCandidateSolitudeHandling(int surfaceParent,int surfaceFollower)
{
    //ensure clean up of coin flipping
    clearToken(TokenType::CandidacyAck,-1);//-1 clears all dirs
    clearToken(TokenType::CandidacyAnnounce,-1);//-1 clears all dirs
    // pass lane 1 solitude lead token forward
    if(peekAtToken(TokenType::SolitudeLeadL1, surfaceFollower) != -1 && canSendToken(TokenType::SolitudeLeadL1, surfaceParent) &&
            canSendToken(TokenType::SolitudeVectorL1, surfaceParent) && canSendToken(TokenType::PassiveSegmentClean, surfaceParent)) {
        int code = receiveToken(TokenType::SolitudeLeadL1, surfaceFollower).value;
        if(code / 100 == 1) { // lead token is on its first lap, so update the lead direction as it's passed
            std::pair<int, int> leadVector = decodeVector(code);
            int offset = (surfaceParent - ((surfaceFollower + 3) % 6) + 6) % 6;
            generateVectorDir = encodeVector(augmentDirVector(leadVector, offset));
            sendToken(TokenType::SolitudeLeadL1, surfaceParent, 100 + generateVectorDir); // lap 1 + new encoded direction vector
        } else { // lead token is on its second pass, just send it forward
            sendToken(TokenType::SolitudeLeadL1, surfaceParent, code);
        }
    }
    // pass lane 2 solitude lead token backward, if doing so does not pass a lane 2 vector token
    if(peekAtToken(TokenType::SolitudeLeadL2, surfaceParent) != -1 && canSendToken(TokenType::SolitudeLeadL2, surfaceFollower) &&
            canSendToken(TokenType::SolitudeVectorL2, surfaceFollower)) {
        int leadValue = receiveToken(TokenType::SolitudeLeadL2, surfaceParent).value;
        sendToken(TokenType::SolitudeLeadL2, surfaceFollower, leadValue);
        if((leadValue % 1000) / 100 == 1) { // first lap
        } else { // second lap
        }
    }
    // if the agent needs to, generate a lane 1 vector token
    if(generateVectorDir != -1 && canSendToken(TokenType::SolitudeVectorL1, surfaceParent) && canSendToken(TokenType::PassiveSegmentClean, surfaceParent)) {
        sendToken(TokenType::SolitudeVectorL1, surfaceParent, generateVectorDir);
        generateVectorDir = -1;
    }
    // perform token matching if there are incoming lane 1 and lane 2 vectors and the various passings are safe
    if(peekAtToken(TokenType::SolitudeVectorL1, surfaceFollower) != -1 && peekAtToken(TokenType::SolitudeVectorL2, surfaceParent) != -1) {
        if(canSendToken(TokenType::SolitudeVectorL1, surfaceParent) && canSendToken(TokenType::SolitudeVectorL2, surfaceFollower) &&
                canSendToken(TokenType::PassiveSegmentClean, surfaceParent)) {
            // decode vectors to do matching on
            std::pair<int, int> vector1 = decodeVector(receiveToken(TokenType::SolitudeVectorL1, surfaceFollower).value);
            std::pair<int, int> vector2 = decodeVector(receiveToken(TokenType::SolitudeVectorL2, surfaceParent).value);
            // do vector token matching; if the components cancel out, then update both sides
            if(vector1.first + vector2.first == 0) {
                vector1.first = 0; vector2.first = 0;
            }
            if(vector1.second + vector2.second == 0) {
                vector1.second = 0; vector2.second = 0;
            }
            // only send the tokens if they are non-(0,0)
            if(vector1 != std::make_pair(0,0)) {
                sendToken(TokenType::SolitudeVectorL1, surfaceParent, encodeVector(vector1));
            }
            if(vector2 != std::make_pair(0,0)) {
                sendToken(TokenType::SolitudeVectorL2, surfaceFollower, encodeVector(vector2));
            }
        }
    } else {
        // if there aren't both lanes of vectors, then pass lane 1 vectors forward...
        if(peekAtToken(TokenType::SolitudeVectorL1, surfaceFollower) != -1 && canSendToken(TokenType::SolitudeVectorL1, surfaceParent) &&
                canSendToken(TokenType::PassiveSegmentClean, surfaceParent)) {
            sendToken(TokenType::SolitudeVectorL1, surfaceParent, receiveToken(TokenType::SolitudeVectorL1, surfaceFollower).value);
        }
        // ...and pass lane 2 vectors backward
        if(peekAtToken(TokenType::SolitudeVectorL2, surfaceParent) != -1 && canSendToken(TokenType::SolitudeVectorL2, surfaceFollower)) {
            sendToken(TokenType::SolitudeVectorL2, surfaceFollower, receiveToken(TokenType::SolitudeVectorL2, surfaceParent).value);
        }
    }
}
void UniversalCoating::nonCandidateCoinFlipping(int surfaceParent, int surfaceFollower)
{
    if(peekAtToken(TokenType::CandidacyAnnounce, surfaceFollower) != -1 && canSendToken(TokenType::CandidacyAnnounce, surfaceParent) &&
            canSendToken(TokenType::PassiveSegmentClean, surfaceParent)) {
        sendToken(TokenType::CandidacyAnnounce, surfaceParent, receiveToken(TokenType::CandidacyAnnounce, surfaceFollower).value);
    }
    // pass acknowledgements backward
    if(peekAtToken(TokenType::CandidacyAck, surfaceParent) != -1 && canSendToken(TokenType::CandidacyAck, surfaceFollower)) {
        sendToken(TokenType::CandidacyAck, surfaceFollower, receiveToken(TokenType::CandidacyAck, surfaceParent).value);
    }
}

int UniversalCoating::encodeVector(std::pair<int, int> vector) const
{
    int x = (vector.first == -1) ? 2 : vector.first;
    int y = (vector.second == -1) ? 2 : vector.second;

    return (10 * x) + y;
}

std::pair<int, int> UniversalCoating::decodeVector(int code)
{
    code = code % 100; // throw out the extra information for lap number
    int x = (code / 10 == 2) ? -1 : code / 10;
    int y = (code % 10 == 2) ? -1 : code % 10;

    return std::make_pair(x, y);
}

std::pair<int, int> UniversalCoating::augmentDirVector(std::pair<int, int> vector, const int offset)
{
    static const std::array<std::pair<int, int>, 6> vectors = {std::make_pair(1,0), std::make_pair(0,1), std::make_pair(-1,1),
                                                               std::make_pair(-1,0), std::make_pair(0,-1), std::make_pair(1,-1)};
    for(auto i = 0; i < vectors.size(); ++i) {
        if(vector == vectors.at(i)) {
            return vectors.at((i + offset) % 6);
        }
    }

    Q_ASSERT(false); // the desired vector should be one of the unit directions
    return std::make_pair(0,0);
}


void UniversalCoating::sendToken(TokenType type, int dir, int valueIn)
{
    if(dir == -1)
        for(int i = 0; i<10;i++)
        {
            outFlags[i].tokens.at((int) type).value  =valueIn;
        }
    else
    {
        outFlags[dir].tokens.at((int) type).value  =valueIn;

    }
}
void UniversalCoating::clearToken(TokenType type, int dir)
{
    if(dir == -1)
        for(int i = 0; i<10;i++)
        {
            outFlags[i].tokens.at((int) type).value  =-1;
            outFlags[i].tokens.at((int)type).receivedToken = false;
        }
    else
    {
        outFlags[dir].tokens.at((int) type).value  =-1;
        outFlags[dir].tokens.at((int)type).receivedToken = false;

    }
}
int UniversalCoating::peekAtToken(TokenType type, int dir) const
{
    if(outFlags[dir].tokens.at((int) type).receivedToken) {
        // if this agent has already read this token, don't peek the same value again
        return -1;
    } else {
        return inFlags[dir]->tokens.at((int) type).value;
    }
}

Token UniversalCoating::receiveToken(TokenType type, int dir)
{
    Q_ASSERT(peekAtToken(type, dir) != -1);
    outFlags[dir].tokens.at((int)type).value = peekAtToken(type, dir);
    outFlags[dir].tokens.at((int) type).receivedToken = true;
    qDebug()<<"recdir "<<(int)type<<" set: "<<outFlags[dir].tokens.at((int)type).value;
    return inFlags[dir]->tokens.at((int) type);
}
//receive all possible tokens in either direction
void UniversalCoating::pipelinePassTokens(int surfaceParent,int surfaceFollower)
{
    qDebug()<<"pipelining from "<<inFlags[surfaceFollower]->id;
    for(auto tokenType : {TokenType::SegmentLead, TokenType::PassiveSegmentClean, TokenType::FinalSegmentClean,
        TokenType::CandidacyAnnounce, TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1, TokenType::BorderTest}) {
        if(peekAtToken(tokenType,surfaceFollower)!=-1) {
            receiveToken(tokenType,surfaceFollower);
            qDebug()<<"  type: "<<(int)tokenType<<" received";
        }

    }
    for(auto tokenType : {TokenType::PassiveSegment, TokenType::ActiveSegment, TokenType::ActiveSegmentClean, TokenType::CandidacyAck,
        TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
        if(peekAtToken(tokenType,surfaceParent)!=-1) {
            receiveToken(tokenType,surfaceParent);
            qDebug()<<"  type: "<<(int)tokenType;
        }

    }
}
void UniversalCoating::tokenCleanup(int surfaceParent,int surfaceFollower)
{
    for(auto tokenType : {TokenType::CandidacyAnnounce}) {
        if(inFlags[surfaceParent]->tokens.at((int) tokenType).receivedToken) {
            outFlags[surfaceParent].tokens.at((int) tokenType).value = -1;
        }
        if(inFlags[surfaceFollower]->tokens.at((int) tokenType).value == -1 && outFlags[surfaceFollower].tokens.at((int) tokenType).receivedToken== true) {
            outFlags[surfaceFollower].tokens.at((int) tokenType).receivedToken = false;
            outFlags[surfaceParent].tokens.at((int) tokenType).value = outFlags[surfaceFollower].tokens.at((int) tokenType).value;
            outFlags[surfaceFollower].tokens.at((int) tokenType).value = -1;
        }
    }
    for(auto tokenType : {TokenType::CandidacyAck}) {
        if(inFlags[surfaceFollower]->tokens.at((int) tokenType).receivedToken) {
            outFlags[surfaceFollower].tokens.at((int) tokenType).value = -1;
        }
        if(inFlags[surfaceParent]->tokens.at((int) tokenType).value == -1 &&  outFlags[surfaceParent].tokens.at((int) tokenType).receivedToken==true) {
            outFlags[surfaceParent].tokens.at((int) tokenType).receivedToken = false;
            outFlags[surfaceFollower].tokens.at((int) tokenType).value = outFlags[surfaceParent].tokens.at((int) tokenType).value;
            outFlags[surfaceParent].tokens.at((int) tokenType).value = -1;
        }
    }

    for(auto tokenType : {TokenType::SegmentLead, TokenType::PassiveSegmentClean, TokenType::FinalSegmentClean,
        TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1, TokenType::BorderTest}) {
        if(inFlags[surfaceParent]->tokens.at((int) tokenType).receivedToken) {
            outFlags[surfaceParent].tokens.at((int) tokenType).value = -1;
        }
        if(inFlags[surfaceFollower]->tokens.at((int) tokenType).value == -1) {
            outFlags[surfaceFollower].tokens.at((int) tokenType).receivedToken = false;
        }
    }
    for(auto tokenType : {TokenType::PassiveSegment, TokenType::ActiveSegment, TokenType::ActiveSegmentClean,
        TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
        if(inFlags[surfaceFollower]->tokens.at((int) tokenType).receivedToken) {
            outFlags[surfaceFollower].tokens.at((int) tokenType).value = -1;
        }
        if(inFlags[surfaceParent]->tokens.at((int) tokenType).value == -1) {
            outFlags[surfaceParent].tokens.at((int) tokenType).receivedToken = false;
        }
    }
}

bool UniversalCoating::canSendToken(TokenType type, int dir) const
{
    return (outFlags[dir].tokens.at((int) type).value == -1 && !inFlags[dir]->tokens.at((int) type).receivedToken);
}

void UniversalCoating::setElectionRole(ElectionRole role)
{
    electionRole = role;
    for(int dir = 0; dir<10;dir++)
    {
        outFlags[dir].electionRole = role;
    }
}

void UniversalCoating::setElectionSubphase(ElectionSubphase subphase)
{
    electionSubphase = subphase;
    for(int dir = 0; dir<10;dir++)
    {
        outFlags[dir].electionSubphase = subphase;
    }
}


void UniversalCoating::updateTokenDirs(int recDir)
{
    int recOppositeDir = (recDir+3)%6;
    int offset =headMarkDir - recOppositeDir;
    if(offset == 5) offset = -1;
    if(offset == -5) offset = 1;
    int tokenCurrentDir = outFlags[0].tokenCurrentDir;
    if(tokenCurrentDir == -1) //uninitialized- so this is axis #1, start counting here
    {
        tokenCurrentDir = 0;
    }
    else
    {
        tokenCurrentDir = (tokenCurrentDir+offset+6)%6;

    }
    for(int label = 0;label<10;label++)
        outFlags[label].tokenCurrentDir = tokenCurrentDir;
    switch(tokenCurrentDir)
    {
    // modify (D1,D2) pair
    case 0:
        for(int label = 0; label<10;label++)
        {
            outFlags[label].tokenD1+=2;
        }
        break;
    case 1:
        for(int label = 0; label<10;label++)
        {
            outFlags[label].tokenD1++;
            outFlags[label].tokenD2++;
        }
        break;
    case 2:
        for(int label = 0; label<10;label++)
        {
            outFlags[label].tokenD1--;
            outFlags[label].tokenD2++;
        }
        break;
    case 3:
        for(int label = 0; label<10;label++)
        {
            outFlags[label].tokenD1-=2;
        }
        break;
    case 4:
        for(int label = 0; label<10;label++)
        {
            outFlags[label].tokenD1--;
            outFlags[label].tokenD2--;
        }
        break;
    case 5:
        for(int label = 0; label<10;label++)
        {
            outFlags[label].tokenD1++;
            outFlags[label].tokenD2--;
        }
        break;
    }
}


void UniversalCoating::setSendingToken(bool value)
{
    for(int label=0; label<10; label++)
        outFlags[label].isSendingToken = value;
}



}
