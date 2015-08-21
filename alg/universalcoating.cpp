
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
      acceptPositionTokens(false)

{
    /*int typenum = 0;
    for(auto token = tokens.begin(); token != tokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        qDebug()<<(int)token->type<<" value "<<token->value;
        ++typenum;
    }
    typenum = 0;
    for(auto token = headPosTokens.begin(); token != headPosTokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        qDebug()<<(int)token->type<<" value "<<token->value;
        ++typenum;
    }

    typenum = 0;
    for(auto token = tailPosTokens.begin(); token != tailPosTokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        qDebug()<<(int)token->type<<" value "<<token->value;
        ++typenum;
    }
*/

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
      headPosTokens(other.headPosTokens),
      tailPosTokens(other.tailPosTokens),
      acceptPositionTokens(other.acceptPositionTokens)
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
    madeAgent = false;
    id = -1;
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
      madeAgent(other.madeAgent),
      id(other.id)
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
            system->insert(Particle(std::make_shared<UniversalCoating>(Phase::Static), randDir(), pos));
            first = false;
        }
        else
        {
            system->insert(Particle(std::make_shared<UniversalCoating>(Phase::Static), randDir(), pos));
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
        system->insert( Particle(std::make_shared<UniversalCoating>(Phase::StaticBorder), randDir(), pos1));

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
        system->insert( Particle(std::make_shared<UniversalCoating>(Phase::StaticBorder), randDir(), pos2));
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
                system->insert(Particle(newParticle, randDir(), *it));
                // system->insert(Particle(std::make_shared<UniversalCoating>(Phase::Inactive), randDir(), *it));
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

Movement UniversalCoating::execute()
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

Movement UniversalCoating::subExecute()
{




    updateNeighborStages();

    if(phase == Phase::Lead || phase == Phase::retiredLeader)
        unsetFollowIndicator();

    if(hasNeighborInPhase(Phase::Static))
    {
        if(phase!=Phase::Inactive )
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

               // setToken(TokenType::PosCandidate);


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
   /* qDebug()<<"id: "<<id;
    qDebug()<<"pos candidate?" <<outFlags[0].headPosTokens.at((int) TokenType::PosCandidate).value<<" "<<
              outFlags[0].tailPosTokens.at((int) TokenType::PosCandidate).value;

    for(int i =0; i<10;i++)
        outFlags[i].id = id;
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
    qDebug()<<"parent: "<<inFlags[surfaceParent]->id<<" follow: "<<inFlags[surfaceFollower]->id;
    if(isExpanded())
    {

    }
    else if(isContracted() )
    {

    }
    else
    {

    }*/
}
void UniversalCoating::setToken(TokenType type)
{
    for(int dir = 0; dir<10;dir++)
    {
        outFlags[dir].headPosTokens.at((int) TokenType::PosCandidate).value  =1;
        //   qDebug()<<(int)TokenType::PosCandidate<<"value set "<<outFlags[dir].tokens.at((int) TokenType::PosCandidate).value<<" for "<<id;
    }
}
void UniversalCoating::copyParentPositionTokens(int surfaceParent)
{
    //copies parent's tail to my head

    if(inFlags[surfaceParent]==nullptr)//behind empty space
        return;
    for(int i =0; i<10;i++)
    {
        for(int tokenIndex = 0; tokenIndex<outFlags[i].tailPosTokens.size();tokenIndex++)
        {
            auto parentTailToken= inFlags[surfaceParent]->tailPosTokens.at(tokenIndex);
            auto headToken =outFlags[i].headPosTokens.at(tokenIndex);
            headToken.type = parentTailToken.type;
            headToken.value = parentTailToken.value;
            headToken.receivedToken = parentTailToken.receivedToken;
        }
    }
}

void UniversalCoating::movePositionTokens(bool toTail)
{
    qDebug()<<"move to tail "<<id;
    //move all head values to tail values
    for(int i =0; i<10;i++)
    {
        if(toTail)
        {
            outFlags[i].tailPosTokens = outFlags[i].headPosTokens;
            /*for(int tokenIndex = 0; tokenIndex<outFlags[i].tailPosTokens.size();tokenIndex++)
            {
                auto tailToken= outFlags[i].tailPosTokens.at(tokenIndex);
                auto headToken =outFlags[i].headPosTokens.at(tokenIndex);
                tailToken.type = headToken.type;
                 tailToken.value = headToken.value;
                 qDebug()<<"moving: "<<(int)tailToken.value<<" from "<<(int)headToken.value;

                tailToken.receivedToken = headToken.receivedToken;
            }*/
        }
    }
    clearPositionTokens(true);//clear out head
}
void UniversalCoating::clearPositionTokens(bool inHead)
{
    int typenum = 0;
    for(int i =0; i<10;i++)
    {
        typenum = 0;
        if(inHead)
        {

            for(auto token = outFlags[i].headPosTokens.begin(); token != outFlags[i].headPosTokens.end(); ++token) {
                token->type = (TokenType) typenum;
                token->value = -1;
                token->receivedToken = false;
                ++typenum;
            }

        }
        else
        {
            for(auto token = outFlags[i].tailPosTokens.begin(); token != outFlags[i].tailPosTokens.end(); ++token) {
                token->type = (TokenType) typenum;
                token->value = -1;
                token->receivedToken = false;
                qDebug()<<(int)token->type<<" value "<<token->value;
                ++typenum;
            }
        }
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
