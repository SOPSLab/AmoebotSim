
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
      electionRole(ElectionRole::None),
      electionSubphase(ElectionSubphase::CoinFlip),
      waitingForTransferAck(false),
      gotAnnounceBeforeAck(false),
      createdLead(false),
      testingBorder(false),
      sawUnmatchedToken(false),
      comparingSegment(false),
      absorbedActiveToken(false),
      isCoveredCandidate(false),
      gotAnnounceInCompare(false),
      generateVectorDir(-1)

{
    int typenum = 0;
    for(auto token = tokens.begin(); token != tokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        qDebug()<<(int)token->type<<" value "<<token->value;
        ++typenum;
    }

    typenum = 0;
    for(auto token = forwardTokens.begin(); token != forwardTokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        qDebug()<<(int)token->type<<" value "<<token->value;
        ++typenum;
    }
    typenum = 0;
    for(auto token = backTokens.begin(); token != backTokens.end(); ++token) {
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
      forwardTokens(other.forwardTokens),
      backTokens(other.backTokens),
      acceptPositionTokens(other.acceptPositionTokens),
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
      gotAnnounceInCompare(other.gotAnnounceInCompare),
      testingBorder(other.testingBorder)

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
    electionRole = ElectionRole::None;
    electionSubphase = ElectionSubphase::CoinFlip;
    waitingForTransferAck = false;
    gotAnnounceBeforeAck = false;
    generateVectorDir = -1;
    createdLead = false;
    sawUnmatchedToken = false;
    comparingSegment= false;
    absorbedActiveToken = false;
    isCoveredCandidate = false;
    gotAnnounceInCompare= false;
    testingBorder =false;


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
      gotAnnounceInCompare(other.gotAnnounceInCompare),
      testingBorder(other.testingBorder)

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
    while(system->getNumParticles() < numStaticParticles) {
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
//<<<<<<< HEAD
  /*  std::shared_ptr<System> system = std::make_shared<System>();
    std::deque<Node> orderedSurface;
    std::set<Node> occupied;
    Node pos;
    int lastOffset = 0;
=======

//    const int hexRadius = 5;
//    const int numParticles = 100;
//    const float holeProb = 0.5;

//    std::shared_ptr<System> system = std::make_shared<System>();

//    std::set<Node> occupied;

//    // grow hexagon of given radius
//    system->insertParticle(Particle(std::make_shared<InfObjCoating>(Phase::Static), randDir(), Node(0, 0)));
//    occupied.insert(Node(0, 0));
//    std::set<Node> layer1, layer2;
//    layer1.insert(Node(0, 0));
//    for(int i = 1; i < hexRadius; i++) {
//        for(auto n : layer1) {
//            for(int dir = 0; dir < 6; dir++) {
//                auto neighbor = n.nodeInDir(dir);
//                if(occupied.find(neighbor) == occupied.end() && layer1.find(neighbor) == layer1.end()) {
//                    layer2.insert(neighbor);
//                }
//            }
//        }

//        for(auto n : layer2) {
//            system->insertParticle(Particle(std::make_shared<InfObjCoating>(Phase::Static), randDir(), n));
//            occupied.insert(n);
//        }

//        layer1 = layer2;
//        layer2.clear();
//    }

//    // determine candidate set by "growing an additional layer"
//    std::set<Node> candidates;
//    for(auto n : layer1) {
//        for(int dir = 0; dir < 6; dir++) {
//            auto neighbor = n.nodeInDir(dir);
//            if(occupied.find(neighbor) == occupied.end() && layer1.find(neighbor) == layer1.end()) {
//                candidates.insert(neighbor);
//            }
//        }
//    }

//    // add inactive particles
//    int numNonStaticParticles = 0;
//    while(numNonStaticParticles < numParticles && !candidates.empty()) {
//        // pick random candidate
//        int randIndex = randInt(0, candidates.size());
//        Node randomCandidate;
//        for(auto it = candidates.begin(); it != candidates.end(); ++it) {
//            if(randIndex == 0) {
//                randomCandidate = *it;
//                candidates.erase(it);
//                break;
//            } else {
//                randIndex--;
//            }
//        }

//        occupied.insert(randomCandidate);

//        if(randBool(1.0f - holeProb)) {
//            // only add particle if not a hole
//            system->insertParticle(Particle(std::make_shared<InfObjCoating>(Phase::Inactive), randDir(), randomCandidate));
//            numNonStaticParticles++;

//            // add new candidates
//            for(int i = 0; i < 6; i++) {
//                auto neighbor = randomCandidate.nodeInDir(i);
//                if(occupied.find(neighbor) == occupied.end()) {
//                    candidates.insert(neighbor);
//                }
//            }
//        }
//    }

//    return system;
}
>>>>>>> 33bc9603045fa48632c128a0046aac4ccd51cf6c

    // begin hexagon structure
    int offset =0;
    int itercount =0;
    int structSideLength=  2;
    int numStructParticles = 6*structSideLength;
    while(system->size() < numStructParticles) {
        system->insert(Particle(std::make_shared<UniversalCoating>(Phase::Static), randDir(), pos));
        occupied.insert(pos);

        orderedSurface.push_back(pos);
        if(itercount%structSideLength ==0)
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
            std::shared_ptr<UniversalCoating> newParticle= std::make_shared<UniversalCoating>(Phase::Inactive);
            newParticle->id = idCounter;
            // if(idCounter!=0 && idCounter!=9)
            // newParticle->ownTokenValue =0;
            system->insert(Particle(newParticle, randDir(), *it));
            numNonStaticParticles++;
            //           qDebug()<<"init? "<<initSide<<"iter: "<<itercount;
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
*/
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
        if(phase!=Phase::Inactive  )
        {
            handlePositionElection();
            if(electionRole == ElectionRole::SoleCandidate)
            {
                auto surfaceFollower = firstNeighborInPhase(Phase::Static);
                int   surfaceParent = headMarkDir;
                if(surfaceFollower!=-1)
                {
                    while(neighborIsInPhase(surfaceFollower,Phase::Static))
                    {
                        surfaceFollower = (surfaceFollower+1)%6;
                    }

                }

                int borderBuildDir = (headMarkDir+1)%6;
                while(neighborIsInPhase(borderBuildDir,Phase::Static) || neighborIsInPhase(borderBuildDir,Phase::StaticBorder))
                    borderBuildDir = (borderBuildDir+5)%6;
                outFlags[borderBuildDir].buildBorder= true;
                qDebug()<<"trying to border?"<<borderBuildDir<<" "<<surfaceParent<<" "<<surfaceFollower;
                setPhase(Phase::Border);
                return Movement(MovementType::Idle);

            }

        }

    }

    if(isExpanded()) {
        setElectionRole(ElectionRole::None);
        if(hasNeighborInPhase(Phase::Static) || hasNeighborInPhase(Phase::StaticBorder))
        {
            bool hasFollower = false;
            for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
                auto label = *it;
                //qDebug()<<"check tail: "<<label;
                if(inFlags[label] != nullptr )
                {
                    hasFollower = true;
                }

            }
            auto surfaceFollower = firstNeighborInPhase(Phase::Static);
            if(surfaceFollower!=-1)
            {
                while(neighborIsInPhase(surfaceFollower,Phase::Static))
                {
                    surfaceFollower = (surfaceFollower+1)%6;
                }

            }
            if(inFlags[surfaceFollower]!=nullptr)
            {
                hasFollower = true;
            }
            if(!hasFollower)
            {
                qDebug()<<"no follow, not contracting";
                return Movement(MovementType::Idle);
            }
        }
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

            if((hasNeighborInPhase(Phase::retiredLeader) || hasNeighborInPhase(Phase::Border))&& phase!=Phase::Lead && phase!=Phase::retiredLeader) {
                if(hasNeighborInPhase(Phase::Border) && !hasNeighborInPhase(Phase::retiredLeader))
                    qDebug()<<"started on top of border";
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

                if(hasNeighborInPhase(Phase::Static))
                {
                    setElectionRole(ElectionRole::Candidate);
                    inFlags[firstNeighborInPhase(Phase::Static)]->electionRole = ElectionRole::Candidate;
                }


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
                    qDebug()<<"hold count: "<<holdCount;
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

bool UniversalCoating::isStatic() const
{
    // NOTE: Alex, I'm not sure what to put here since you have two different kinds of static.
    return false;
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

    Q_ASSERT(hasNeighborInPhase(Phase::Static));
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
            inFlags[surfaceFollower] ==nullptr || inFlags[surfaceParent]==nullptr || inFlags[surfaceParent]->isExpanded() ||inFlags[surfaceFollower]->isExpanded()
           )
    {
        return;
    }

    qDebug()<<"id: "<<id<<"parent: "<<inFlags[surfaceParent]->id<<" follow: "<<inFlags[surfaceFollower]->id<<" "<<isExpanded();
    if(isExpanded())//copy up values but for now don't act on them
    {

    }
    else
    {
        //copy up


        int staticRecDir = surfaceFollower;//always same for anyone in this position
        while(!neighborIsInPhase(staticRecDir,Phase::Static))
            staticRecDir = (staticRecDir+5)%6;
        //copy tokens for forward and backward
        outFlags[surfaceParent].tokens = inFlags[staticRecDir]->forwardTokens;
        outFlags[surfaceFollower].tokens = inFlags[staticRecDir]->backTokens;
        //methods put to local + outFlag copies
        setElectionRole(inFlags[staticRecDir]->electionRole);
        setElectionSubphase(inFlags[staticRecDir]->electionSubphase);
        //other info is only for local use, doesn't have to go to outflags
        waitingForTransferAck = inFlags[staticRecDir]->waitingForTransferAck;
        gotAnnounceBeforeAck = inFlags[staticRecDir]->gotAnnounceBeforeAck;
        generateVectorDir = inFlags[staticRecDir]->generateVectorDir;
        createdLead = inFlags[staticRecDir]->createdLead;
        sawUnmatchedToken = inFlags[staticRecDir]->sawUnmatchedToken;
        comparingSegment= inFlags[staticRecDir]->comparingSegment;
        absorbedActiveToken = inFlags[staticRecDir]->absorbedActiveToken;
        isCoveredCandidate = inFlags[staticRecDir]->isCoveredCandidate;
        gotAnnounceInCompare= inFlags[staticRecDir]->gotAnnounceInCompare;
        testingBorder = inFlags[staticRecDir]->testingBorder;
        qDebug()<<"read role: "<<(int)electionRole<<" subphase: "<<(int)electionSubphase;
        //act
        if(electionRole == ElectionRole::None)
            electionRole = ElectionRole::Demoted;
        if( inFlags[surfaceParent]->electionRole != ElectionRole::None && inFlags[surfaceFollower]->electionRole != ElectionRole::None)
                  ExecuteLeaderElection(surfaceFollower,surfaceParent);
        printTokens(surfaceFollower,surfaceParent);
        //copy down
        inFlags[staticRecDir]->forwardTokens =  outFlags[surfaceParent].tokens;
        inFlags[staticRecDir]->backTokens =  outFlags[surfaceFollower].tokens ;
        inFlags[staticRecDir]->electionSubphase = electionSubphase;
        inFlags[staticRecDir]->electionRole = electionRole;
        //other info is only for local use, doesn't have to go to outflags
        inFlags[staticRecDir]-> waitingForTransferAck = waitingForTransferAck;
        inFlags[staticRecDir]-> gotAnnounceBeforeAck = gotAnnounceBeforeAck;
        inFlags[staticRecDir]-> generateVectorDir = generateVectorDir;
        inFlags[staticRecDir]-> createdLead =createdLead;
        inFlags[staticRecDir]->sawUnmatchedToken = sawUnmatchedToken;
        inFlags[staticRecDir]->comparingSegment= comparingSegment;
        inFlags[staticRecDir]-> absorbedActiveToken =absorbedActiveToken;
        inFlags[staticRecDir]->isCoveredCandidate = isCoveredCandidate;
        inFlags[staticRecDir]->gotAnnounceInCompare= gotAnnounceInCompare;
        inFlags[staticRecDir]->testingBorder = testingBorder;
        qDebug()<<"wrote role: "<<(int)electionRole<<" subphase: "<<(int)electionSubphase;
    }
}
void UniversalCoating::ExecuteLeaderElection(int prevAgentDir, int nextAgentDir)
{
    tokenCleanup(prevAgentDir,nextAgentDir);
    if(electionRole == ElectionRole::Candidate) {
        /* this first block of tasks should be performed by any candidate, regardless of subphase */
        // if there is an incoming active segment cleaning token, consume it
        if(peekAtToken(TokenType::ActiveSegmentClean, nextAgentDir) != -1) {
            performActiveClean(2, prevAgentDir, nextAgentDir); // clean the front side only
            receiveToken(TokenType::ActiveSegmentClean, nextAgentDir);
        }
        // if there is an incoming passive segment cleaning token, consume it
        if(peekAtToken(TokenType::PassiveSegmentClean, prevAgentDir) != -1) {
            performPassiveClean(1, prevAgentDir, nextAgentDir); // clean the back side only
            receiveToken(TokenType::PassiveSegmentClean, prevAgentDir);
            paintBackSegment(QColor("dimgrey").rgb());
        }
        // if there is an incoming segment lead token, consume it and generate the final passive token
        if(peekAtToken(TokenType::SegmentLead, prevAgentDir) != -1 && canSendToken(TokenType::PassiveSegment, prevAgentDir)) {
            receiveToken(TokenType::SegmentLead, prevAgentDir);
            sendToken(TokenType::PassiveSegment, prevAgentDir, 2); // 2 => last passive/active token
            paintBackSegment(QColor("dimgrey").rgb());
        }
        // if there is an incoming active segment token, either absorb it and acknowledge covering or pass it backward
        if(peekAtToken(TokenType::ActiveSegment, nextAgentDir) != -1) {
            if(!absorbedActiveToken) {
                if(receiveToken(TokenType::ActiveSegment, nextAgentDir).value == 2) { // absorbing the last active token
                    sendToken(TokenType::FinalSegmentClean, nextAgentDir, 2);
                } else { // this candidate is now covered
                    Q_ASSERT(canSendToken(TokenType::ActiveSegmentClean, prevAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir));
                    sendToken(TokenType::PassiveSegmentClean, nextAgentDir, 1);
                    performPassiveClean(2, prevAgentDir, nextAgentDir); // clean the front side only
                    paintFrontSegment(QColor("dimgrey").rgb());
                    sendToken(TokenType::ActiveSegmentClean, prevAgentDir, 1);
                    performActiveClean(1, prevAgentDir, nextAgentDir); // clean the back side only
                    absorbedActiveToken = true;
                    isCoveredCandidate = true;
                    setElectionRole(ElectionRole::Demoted);
                    return; // completed subphase and thus shouldn't perform any more operations in this round
                }
            } else if(canSendToken(TokenType::ActiveSegment, prevAgentDir) && canSendToken(TokenType::ActiveSegmentClean, prevAgentDir)) {
                sendToken(TokenType::ActiveSegment, prevAgentDir, receiveToken(TokenType::ActiveSegment, nextAgentDir).value);
            }
        }

        // if there is an announcement waiting to be received by me and it is safe to create an acknowledgement, consume the announcement
        if(peekAtToken(TokenType::CandidacyAnnounce, prevAgentDir) != -1 && canSendToken(TokenType::CandidacyAck, prevAgentDir)) {
            receiveToken(TokenType::CandidacyAnnounce, prevAgentDir);
            sendToken(TokenType::CandidacyAck, prevAgentDir, 1);
            paintBackSegment(QColor("dimgrey").rgb());
            if(waitingForTransferAck) {
                gotAnnounceBeforeAck = true;
            } else if(comparingSegment) {
                gotAnnounceInCompare = true;
            }
        }

        // if there is a solitude lead token waiting to be put into lane 2, put it there if it doesn't pass a vector token
        if(peekAtToken(TokenType::SolitudeLeadL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL2, prevAgentDir) &&
                canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
            int leadValue = receiveToken(TokenType::SolitudeLeadL1, prevAgentDir).value;
            if(leadValue / 100 == 1) { // if the lead is on lap 1, append this candidate's local id
                sendToken(TokenType::SolitudeLeadL2, prevAgentDir, (1000 * 1) + leadValue);
                paintBackSegment(QColor("deepskyblue").rgb());
            } else { // if the lead is on lap 2, just pass on the value
                sendToken(TokenType::SolitudeLeadL2, prevAgentDir, leadValue);
                paintBackSegment(QColor("dimgrey").rgb());
            }
        }
        // if there is a vector waiting to be put into lane 2, put it there
        if(peekAtToken(TokenType::SolitudeVectorL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
            sendToken(TokenType::SolitudeVectorL2, prevAgentDir, receiveToken(TokenType::SolitudeVectorL1, prevAgentDir).value);
        }

        /* the next block of tasks are dependent upon subphase */
        if(electionSubphase == ElectionSubphase::SegmentComparison) {
            if(peekAtToken(TokenType::PassiveSegment, nextAgentDir) != -1 && canSendToken(TokenType::ActiveSegment, prevAgentDir)) {
                // if there is an incoming passive token, pass it on as an active token
                int passiveValue = receiveToken(TokenType::PassiveSegment, nextAgentDir).value;
                sendToken(TokenType::ActiveSegment, prevAgentDir, passiveValue);
                if(passiveValue == 2) {
                    paintFrontSegment(QColor("dimgrey").rgb());
                }
            } else if(peekAtToken(TokenType::FinalSegmentClean, prevAgentDir) != -1) {
                // if there is an incoming final cleaning token, consume it and proceed according to its value
                int finalCleanValue = receiveToken(TokenType::FinalSegmentClean, prevAgentDir).value;
                if(finalCleanValue == 0 && !gotAnnounceInCompare) { // if this candidate did not cover any tokens and was not transferred candidacy, demote
                    setElectionRole(ElectionRole::Demoted);
                } else {
                    setElectionSubphase(ElectionSubphase::CoinFlip);
                }
                comparingSegment = false;
                gotAnnounceInCompare = false;
                return; // completed subphase and thus shouldn't perform any more operations in this round
            } else if(!comparingSegment) {
                // begin segment comparison by generating a segment lead token
                Q_ASSERT(canSendToken(TokenType::SegmentLead, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir));
                sendToken(TokenType::SegmentLead, nextAgentDir, 1);
                paintFrontSegment(QColor("red").rgb());
                comparingSegment = true;
            }
        } else if(electionSubphase == ElectionSubphase::CoinFlip) {
            if(peekAtToken(TokenType::CandidacyAck, nextAgentDir) != -1) {
                // if there is an acknowledgement waiting, consume the acknowledgement and proceed to the next subphase
                receiveToken(TokenType::CandidacyAck, nextAgentDir);
                paintFrontSegment(QColor("dimgrey").rgb());
                setElectionSubphase(ElectionSubphase::SolitudeVerification);
                if(!gotAnnounceBeforeAck) {
                    setElectionRole(ElectionRole::Demoted);
                }
                waitingForTransferAck = false;
                gotAnnounceBeforeAck = false;
                return; // completed subphase and thus shouldn't perform any more operations in this round
            } else if(!waitingForTransferAck && randBool()) {
                // if I am not waiting for an acknowlegdement of my previous announcement and I win the coin flip, announce a transfer of candidacy
                Q_ASSERT(canSendToken(TokenType::CandidacyAnnounce, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)); // there shouldn't be a call to make two announcements
                sendToken(TokenType::CandidacyAnnounce, nextAgentDir, 1);
                paintFrontSegment(QColor("orange").rgb());
                waitingForTransferAck = true;
            }
        } else if(electionSubphase == ElectionSubphase::SolitudeVerification) {
            // if the agent needs to, generate a lane 1 vector token
            if(generateVectorDir != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                sendToken(TokenType::SolitudeVectorL1, nextAgentDir, generateVectorDir);
                generateVectorDir = -1;
            }

            if(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != -1) {
                // consume all vector tokens that have not been matched, decide that solitude has failed
                Q_ASSERT(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != 0); // matched tokens should have dropped
                receiveToken(TokenType::SolitudeVectorL2, nextAgentDir);
                sawUnmatchedToken = true;
            } else if(peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) != -1) {
                // if the lead token has returned, either put it back in lane 1 (lap 1) or consume it and decide solitude (lap 2)
                if((peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) % 1000) / 100 == 1) { // lead has just completed lap 1
                    if(canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) &&
                            canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                        int leadValue = receiveToken(TokenType::SolitudeLeadL2, nextAgentDir).value;
                        sendToken(TokenType::SolitudeLeadL1, nextAgentDir, (leadValue / 1000) * 1000 + 200); // lap 2, orientation no longer matters => 200
                        paintFrontSegment(QColor("deepskyblue").lighter().rgb());
                    }
                } else if((peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) % 1000) / 100 == 2) { // lead has just completed lap 2
                    int leadValue = receiveToken(TokenType::SolitudeLeadL2, nextAgentDir).value;
                    paintFrontSegment(QColor("dimgrey").rgb());
                    if(!sawUnmatchedToken && (leadValue / 1000) == 1) { // if it did not consume an unmatched token and it assures it's matching with itself, go to inner/outer test
                        setElectionRole(ElectionRole::SoleCandidate);
                    } else { // if solitude verification failed, then do another coin flip compeititon
                        setElectionSubphase(ElectionSubphase::SegmentComparison);
                    }
                    createdLead = false;
                    sawUnmatchedToken = false;
                    return; // completed subphase and thus shouldn't perform any more operations in this round
                }
            } else if(!createdLead) {
                // to begin the solitude verification, create a lead token with an orientation to communicate to its segment
                Q_ASSERT(canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) &&
                         canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)); // there shouldn't be a call to make two leads
                sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 100 + encodeVector(std::make_pair(1,0))); // lap 1 in direction (1,0)
                paintFrontSegment(QColor("deepskyblue").darker().rgb());
                createdLead = true;
                generateVectorDir = encodeVector(std::make_pair(1,0));
            }
        }
    } else if(electionRole == ElectionRole::SoleCandidate) {
        if(!testingBorder) { // begin the inner/outer border test
            Q_ASSERT(canSendToken(TokenType::BorderTest, nextAgentDir));
            sendToken(TokenType::BorderTest, nextAgentDir, addNextBorder(0, prevAgentDir, nextAgentDir));
            paintFrontSegment(-1);
            testingBorder = true;
        } else if(peekAtToken(TokenType::BorderTest, prevAgentDir) != -1) { // test is complete
            int borderSum = receiveToken(TokenType::BorderTest, prevAgentDir).value;
            paintBackSegment(-1);
            if(borderSum == 1) { // outer border, agent becomes the leader
                setElectionRole(ElectionRole::Leader);
            } else if(borderSum == 4) { // inner border, demote agent and set to finished
                setElectionRole(ElectionRole::Demoted);
            }
            testingBorder = false;
            return; // completed subphase and thus shouldn't perform any more operations in this round
        }
    } else if(electionRole == ElectionRole::Demoted) {
        // SUBPHASE: Segment Comparison
        // pass passive segment cleaning tokens forward, and perform cleaning
        if(peekAtToken(TokenType::PassiveSegmentClean, prevAgentDir) != -1 && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
            performPassiveClean(3, prevAgentDir, nextAgentDir); // clean the full segment
            sendToken(TokenType::PassiveSegmentClean, nextAgentDir, receiveToken(TokenType::PassiveSegmentClean, prevAgentDir).value);
            paintFrontSegment(QColor("dimgrey").rgb()); paintBackSegment(QColor("dimgrey").rgb());
        }
        // pass active segment cleaning tokens backward, and perform cleaning
        if(peekAtToken(TokenType::ActiveSegmentClean, nextAgentDir) != -1 && canSendToken(TokenType::ActiveSegmentClean, prevAgentDir)) {
            performActiveClean(3, prevAgentDir, nextAgentDir); // clean the full segment
            sendToken(TokenType::ActiveSegmentClean, prevAgentDir, receiveToken(TokenType::ActiveSegmentClean, nextAgentDir).value);
        }
        // if there is an incoming segment lead token, pass it forward and generate a passive token
        if(peekAtToken(TokenType::SegmentLead, prevAgentDir) != -1 && canSendToken(TokenType::SegmentLead, nextAgentDir) &&
                canSendToken(TokenType::PassiveSegment, prevAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
            sendToken(TokenType::SegmentLead, nextAgentDir, receiveToken(TokenType::SegmentLead, prevAgentDir).value);
            sendToken(TokenType::PassiveSegment, prevAgentDir, 1); // 1 => usual passive/active token
            paintFrontSegment(QColor("red").rgb()); paintBackSegment(QColor("red").rgb());
        }
        // pass passive tokens backward
        if(peekAtToken(TokenType::PassiveSegment, nextAgentDir) != -1 && canSendToken(TokenType::PassiveSegment, prevAgentDir)) {
            int passiveValue = receiveToken(TokenType::PassiveSegment, nextAgentDir).value;
            sendToken(TokenType::PassiveSegment, prevAgentDir, passiveValue);
            if(passiveValue == 2) {
                paintFrontSegment(QColor("dimgrey").rgb()); paintBackSegment(QColor("dimgrey").rgb());
            }
        }
        // either absorb active tokens or pass them backward (but don't pass an active cleaning token)
        if(peekAtToken(TokenType::ActiveSegment, nextAgentDir) != -1) {
            // absorb the token if possible
            if(!absorbedActiveToken) {
                if(receiveToken(TokenType::ActiveSegment, nextAgentDir).value == 2) { // if absorbing the final active token, generate the final cleaning token
                    sendToken(TokenType::FinalSegmentClean, nextAgentDir, 0); // the final segment cleaning token begins as having not seen covered candidates
                } else {
                    absorbedActiveToken = true;
                }
            } else if(canSendToken(TokenType::ActiveSegment, prevAgentDir) && canSendToken(TokenType::ActiveSegmentClean, prevAgentDir)) {
                // pass active token backward if doing so does not pass the active cleaning token
                sendToken(TokenType::ActiveSegment, prevAgentDir, receiveToken(TokenType::ActiveSegment, nextAgentDir).value);
            }
        }
        // pass final cleaning token forward, perform cleaning, and check for covered candidates
        if(peekAtToken(TokenType::FinalSegmentClean, prevAgentDir) != -1 && canSendToken(TokenType::FinalSegmentClean, nextAgentDir)) {
            int finalCleanValue = receiveToken(TokenType::FinalSegmentClean, prevAgentDir).value;
            if(finalCleanValue != 2 && isCoveredCandidate) {
                finalCleanValue = 1;
            }
            absorbedActiveToken = false;
            isCoveredCandidate = false;
            sendToken(TokenType::FinalSegmentClean, nextAgentDir, finalCleanValue);
        }

        // SUBPHASE: Coin Flipping
        // pass announcements forward
        if(peekAtToken(TokenType::CandidacyAnnounce, prevAgentDir) != -1 && canSendToken(TokenType::CandidacyAnnounce, nextAgentDir) &&
                canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
            sendToken(TokenType::CandidacyAnnounce, nextAgentDir, receiveToken(TokenType::CandidacyAnnounce, prevAgentDir).value);
            paintFrontSegment(QColor("orange").rgb()); paintBackSegment(QColor("orange").rgb());
        }
        // pass acknowledgements backward
        if(peekAtToken(TokenType::CandidacyAck, nextAgentDir) != -1 && canSendToken(TokenType::CandidacyAck, prevAgentDir)) {
            sendToken(TokenType::CandidacyAck, prevAgentDir, receiveToken(TokenType::CandidacyAck, nextAgentDir).value);
            paintFrontSegment(QColor("dimgrey").rgb()); paintBackSegment(QColor("dimgrey").rgb());
        }

        // SUBPHASE: Solitude Verification
        // pass lane 1 solitude lead token forward
        if(peekAtToken(TokenType::SolitudeLeadL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) &&
                canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
            int code = receiveToken(TokenType::SolitudeLeadL1, prevAgentDir).value;
            if(code / 100 == 1) { // lead token is on its first lap, so update the lead direction as it's passed
                std::pair<int, int> leadVector = decodeVector(code);
                int offset = (nextAgentDir - ((prevAgentDir + 3) % 6) + 6) % 6;
                generateVectorDir = encodeVector(augmentDirVector(leadVector, offset));
                sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 100 + generateVectorDir); // lap 1 + new encoded direction vector
                paintFrontSegment(QColor("deepskyblue").darker().rgb()); paintBackSegment(QColor("deepskyblue").darker().rgb());
            } else { // lead token is on its second pass, just send it forward
                sendToken(TokenType::SolitudeLeadL1, nextAgentDir, code);
                paintFrontSegment(QColor("deepskyblue").lighter().rgb()); paintBackSegment(QColor("deepskyblue").lighter().rgb());
            }
        }
        // pass lane 2 solitude lead token backward, if doing so does not pass a lane 2 vector token
        if(peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL2, prevAgentDir) &&
                canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
            int leadValue = receiveToken(TokenType::SolitudeLeadL2, nextAgentDir).value;
            sendToken(TokenType::SolitudeLeadL2, prevAgentDir, leadValue);
            if((leadValue % 1000) / 100 == 1) { // first lap
                paintFrontSegment(QColor("deepskyblue").rgb()); paintBackSegment(QColor("deepskyblue").rgb());
            } else { // second lap
                paintFrontSegment(QColor("dimgrey").rgb()); paintBackSegment(QColor("dimgrey").rgb());
            }
        }
        // if the agent needs to, generate a lane 1 vector token
        if(generateVectorDir != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
            sendToken(TokenType::SolitudeVectorL1, nextAgentDir, generateVectorDir);
            generateVectorDir = -1;
        }
        // perform token matching if there are incoming lane 1 and lane 2 vectors and the various passings are safe
        if(peekAtToken(TokenType::SolitudeVectorL1, prevAgentDir) != -1 && peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != -1) {
            if(canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::SolitudeVectorL2, prevAgentDir) &&
                    canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                // decode vectors to do matching on
                std::pair<int, int> vector1 = decodeVector(receiveToken(TokenType::SolitudeVectorL1, prevAgentDir).value);
                std::pair<int, int> vector2 = decodeVector(receiveToken(TokenType::SolitudeVectorL2, nextAgentDir).value);
                // do vector token matching; if the components cancel out, then update both sides
                if(vector1.first + vector2.first == 0) {
                    vector1.first = 0; vector2.first = 0;
                }
                if(vector1.second + vector2.second == 0) {
                    vector1.second = 0; vector2.second = 0;
                }
                // only send the tokens if they are non-(0,0)
                if(vector1 != std::make_pair(0,0)) {
                    sendToken(TokenType::SolitudeVectorL1, nextAgentDir, encodeVector(vector1));
                }
                if(vector2 != std::make_pair(0,0)) {
                    sendToken(TokenType::SolitudeVectorL2, prevAgentDir, encodeVector(vector2));
                }
            }
        } else {
            // if there aren't both lanes of vectors, then pass lane 1 vectors forward...
            if(peekAtToken(TokenType::SolitudeVectorL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) &&
                    canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                sendToken(TokenType::SolitudeVectorL1, nextAgentDir, receiveToken(TokenType::SolitudeVectorL1, prevAgentDir).value);
            }
            // ...and pass lane 2 vectors backward
            if(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != -1 && canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
                sendToken(TokenType::SolitudeVectorL2, prevAgentDir, receiveToken(TokenType::SolitudeVectorL2, nextAgentDir).value);
            }
        }

        // SUBPHASE: Inner/Outer Border Test
        if(peekAtToken(TokenType::BorderTest, prevAgentDir) != -1 && canSendToken(TokenType::BorderTest, nextAgentDir)) {
            int borderSum = addNextBorder(receiveToken(TokenType::BorderTest, prevAgentDir).value, prevAgentDir, nextAgentDir);
            sendToken(TokenType::BorderTest, nextAgentDir, borderSum);
            paintFrontSegment(-1); paintBackSegment(-1);
            setElectionRole(ElectionRole::Demoted);
        }
    }
}

void UniversalCoating::paintFrontSegment(const int color)
{
}

void UniversalCoating::paintBackSegment(const int color)
{
}

bool UniversalCoating::canSendToken(TokenType type, int dir) const
{
    return (outFlags[dir].tokens.at((int) type).value == -1 && !inFlags[dir]->tokens.at((int) type).receivedToken);
}

void UniversalCoating::sendToken(TokenType type, int dir, int value)
{

    Q_ASSERT(canSendToken(type, dir));
    outFlags[dir].tokens.at((int) type).value = value;
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
    qDebug()<<"receive: "<<(int)type<< peekAtToken(type, dir);
    outFlags[dir].tokens.at((int) type).receivedToken = true;
    return inFlags[dir]->tokens.at((int) type);
}

void UniversalCoating::tokenCleanup(int prevAgentDir, int nextAgentDir)
{
    for(auto tokenType : {TokenType::SegmentLead, TokenType::PassiveSegmentClean, TokenType::FinalSegmentClean,
        TokenType::CandidacyAnnounce, TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1, TokenType::BorderTest}) {
        if(inFlags[nextAgentDir]->tokens.at((int) tokenType).receivedToken) {
            outFlags[nextAgentDir].tokens.at((int) tokenType).value = -1;
        }
        if(inFlags[prevAgentDir]->tokens.at((int) tokenType).value == -1) {
            outFlags[prevAgentDir].tokens.at((int) tokenType).receivedToken = false;
        }
    }
    for(auto tokenType : {TokenType::PassiveSegment, TokenType::ActiveSegment, TokenType::ActiveSegmentClean, TokenType::CandidacyAck,
        TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
        if(inFlags[prevAgentDir]->tokens.at((int) tokenType).receivedToken) {
            outFlags[prevAgentDir].tokens.at((int) tokenType).value = -1;
        }
        if(inFlags[nextAgentDir]->tokens.at((int) tokenType).value == -1) {
            outFlags[nextAgentDir].tokens.at((int) tokenType).receivedToken = false;
        }
    }
}

void UniversalCoating::performPassiveClean(const int region, int prevAgentDir, int nextAgentDir)
{
    Q_ASSERT(1 <= region && region <= 3); // 1 => back, 2 => front, 3 => both
    if(region != 2) { // back
        for(auto tokenType : {TokenType::SegmentLead, TokenType::CandidacyAnnounce, TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1}) {
            if(peekAtToken(tokenType, prevAgentDir) != -1) {
                receiveToken(tokenType, prevAgentDir);
            }
        }
    }
    if(region != 1) { // front
        for(auto tokenType : {TokenType::PassiveSegment, TokenType::CandidacyAck, TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
            if(peekAtToken(tokenType, nextAgentDir) != -1) {
                receiveToken(tokenType, nextAgentDir);
            }
        }
    }
}

void UniversalCoating::performActiveClean(const int region, int prevAgentDir, int nextAgentDir)
{
    Q_ASSERT(1 <= region && region <= 3); // 1 => back, 2 => front, 3 => both
    if(region != 2 && peekAtToken(TokenType::FinalSegmentClean, prevAgentDir) != -1) { // back
        receiveToken(TokenType::FinalSegmentClean, prevAgentDir);
    }
    if(region != 1 && peekAtToken(TokenType::ActiveSegment, nextAgentDir) != -1) { // front
        receiveToken(TokenType::ActiveSegment, nextAgentDir);
    }
    absorbedActiveToken = false;
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

int UniversalCoating::addNextBorder(int currentSum, int prevAgentDir, int nextAgentDir)
{
    // adjust offset in modulo 6 to be compatible with modulo 5 computations
    int offsetMod6 = (prevAgentDir + 3) % 6 - nextAgentDir;
    if(4 <= offsetMod6 && offsetMod6 <= 5) {
        offsetMod6 -= 6;
    } else if(-5 <= offsetMod6 && offsetMod6 <= -3) {
        offsetMod6 += 6;
    }

    return (currentSum + offsetMod6 + 5) % 5;
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





void UniversalCoating::printTokens(int prevAgentDir, int nextAgentDir)
{
    qDebug()<<"forward out: ";
    for(auto tokenType : {TokenType::SegmentLead, TokenType::PassiveSegmentClean, TokenType::FinalSegmentClean,
        TokenType::CandidacyAnnounce, TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1, TokenType::BorderTest, TokenType::PassiveSegment, TokenType::ActiveSegment, TokenType::ActiveSegmentClean, TokenType::CandidacyAck,
        TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
        if(outFlags[nextAgentDir].tokens.at((int) tokenType).receivedToken) {
            qDebug()<<"  token "<<(int)tokenType<<" received";
        }
        if(outFlags[nextAgentDir].tokens.at((int) tokenType).value != -1) {
            qDebug()<<"  token "<<(int)tokenType<<" value: "<<outFlags[nextAgentDir].tokens.at((int) tokenType).value;
        }
    }
    qDebug()<<"backward out: ";
    for(auto tokenType : {TokenType::SegmentLead, TokenType::PassiveSegmentClean, TokenType::FinalSegmentClean,
        TokenType::CandidacyAnnounce, TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1, TokenType::BorderTest, TokenType::PassiveSegment, TokenType::ActiveSegment, TokenType::ActiveSegmentClean, TokenType::CandidacyAck,
        TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
        if(outFlags[prevAgentDir].tokens.at((int) tokenType).receivedToken) {
            qDebug()<<"  token "<<(int)tokenType<<" received";
        }
        if(outFlags[prevAgentDir].tokens.at((int) tokenType).value != -1) {
            qDebug()<<"  token "<<(int)tokenType<<" value: "<<outFlags[prevAgentDir].tokens.at((int) tokenType).value;
        }
    }



}

}
