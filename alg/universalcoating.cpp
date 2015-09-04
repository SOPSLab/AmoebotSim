
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

      tokenCurrentDir(-1),
      isSendingToken(false),
      ownTokenValue(-1),
      buildBorder(false),
      acceptPositionTokens(false),
      id(-1),
      status("")
{
    // init: all 4 tokens sets
    int typenum = 0;
    for(auto token = contrLocData.backTokens.begin(); token != contrLocData.backTokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        ++typenum;
    }
    typenum = 0;
    for(auto token = contrLocData.forwardTokens.begin(); token != contrLocData.forwardTokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        ++typenum;
    }
    typenum = 0;
    for(auto token = expLocData.backTokens.begin(); token != expLocData.backTokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        ++typenum;
    }
    typenum = 0;
    for(auto token = expLocData.forwardTokens.begin(); token != expLocData.forwardTokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        ++typenum;
    }

    contrLocData.electionRole = ElectionRole::Demoted;
    expLocData.electionRole = ElectionRole::Demoted;

    contrLocData.electionSubphase = ElectionSubphase::SegmentComparison;
    expLocData.electionSubphase = ElectionSubphase::SegmentComparison;


    for(int i = 0; i < contrLocData.switches.size(); i++) {
        contrLocData.switches.at(i) = 0;
    }
    for(int i = 0; i < expLocData.switches.size(); i++) {
        expLocData.switches.at(i) = 0;
    }
    contrLocData.switches.at((int)SwitchVariable::generateVectorDir) = -1;
    expLocData.switches.at((int)SwitchVariable::generateVectorDir) = -1;

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
      tokenCurrentDir(other.tokenCurrentDir),
      isSendingToken(other.isSendingToken),
      ownTokenValue(other.ownTokenValue),
      buildBorder(other.buildBorder),
      id(other.id),
      expLocData(other.expLocData),
      contrLocData(other.contrLocData),
      status(other.status)

{}

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
    sentBorder =false;
    startedRetired =false;
    cleanHeadLocData();
    cleanTailLocData();
    memcpy(&contrLocData ,&tailLocData,sizeof(tailLocData));


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
      headLocData(other.headLocData),
      tailLocData(other.tailLocData),
      contrLocData(other.contrLocData),
      sentBorder(other.sentBorder),
      startedRetired(other.startedRetired)

{}

UniversalCoating::~UniversalCoating()
{}

std::shared_ptr<System> UniversalCoating::instance(const int staticParticlesRadius, const int numParticles, const float holeProb)
{
    std::shared_ptr<System> system = std::make_shared<System>();
    std::set<Node> occupied;
    const int hexRadius = staticParticlesRadius;

    // grow hexagon of given radius
    system->insertParticle(Particle(std::make_shared<UniversalCoating>(Phase::Static), randDir(), Node(0, 0)));
    occupied.insert(Node(0, 0));
    std::set<Node> layer1, layer2;
    layer1.insert(Node(0, 0));
    for(int i = 1; i < hexRadius; i++) {
        for(auto n : layer1) {
            for(int dir = 0; dir < 6; dir++) {
                auto neighbor = n.nodeInDir(dir);
                if(occupied.find(neighbor) == occupied.end() && layer1.find(neighbor) == layer1.end()) {
                    layer2.insert(neighbor);
                }
            }
        }

        for(auto n : layer2) {
            system->insertParticle(Particle(std::make_shared<UniversalCoating>(Phase::Static), randDir(), n));
            occupied.insert(n);
        }

        layer1 = layer2;
        layer2.clear();
    }

    // determine candidate set by "growing an additional layer"
    std::set<Node> candidates;
    for(auto n : layer1) {
        for(int dir = 0; dir < 6; dir++) {
            auto neighbor = n.nodeInDir(dir);
            if(occupied.find(neighbor) == occupied.end() && layer1.find(neighbor) == layer1.end()) {
                candidates.insert(neighbor);
            }
        }
    }

    // add inactive particles
    int particleID = 0;
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
            std::shared_ptr<UniversalCoating> newParticle= std::make_shared<UniversalCoating>(Phase::Inactive);
            newParticle->id = particleID;
            particleID++;
            system->insertParticle(Particle(newParticle, randDir(), randomCandidate));
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

    return system;
}


// wrapper to capture motion for token storage moving stuff before actually returning it
Movement UniversalCoating::execute()
{
    for(int i =0; i<10;i++)
        outFlags[i].id = id;

    qDebug()<<"id: "<<id;
    qDebug()<<"expanded? "<<isExpanded();
    int headMarkDirBefore = headMarkDir;

    //vars for determining if to return empty
    sentBorder = false;
    if(phase==Phase::retiredLeader)
        startedRetired = true;
    else
        startedRetired =false;

    //determine surfaceParent, the direction toward the next particle counterclockwise around the surface, and surfaceFollow, the direction toward the previous
    int surfaceFollow = getSurfaceFollowDir();
    int surfaceParent = getSurfaceParentDir();
    if(surfaceParent==-1 && !hasNeighborInPhase(Phase::Static))
        surfaceParent = headMarkDir;

     bool hasParent = false;
     bool hasFollow = false;
    if(surfaceParent!=-1 && inFlags[surfaceParent]!=nullptr)
    {

        std::string parentStatus = inFlags[surfaceParent]->status.c_str();
        if(inFlags[surfaceParent]->isContracted())//if contracted, will read from contracted channel regardless
        {
            parentStatus ="C";
        }
        QString qstr(parentStatus.c_str());

        qDebug()<<"parent: id:"<<inFlags[surfaceParent]->id<<" status: "<<qstr<<" contracted? "<<inFlags[surfaceParent]->isContracted();
        hasParent = true;
    }
    if(surfaceFollow!=-1 && inFlags[surfaceFollow]!=nullptr)
    {
        std::string followStatus = inFlags[surfaceFollow]->status.c_str();
        if(inFlags[surfaceFollow]->isContracted())//if contracted, will read from contracted channel regardless
        {
            followStatus ="C";
        }
        QString qstr(followStatus.c_str());

        qDebug()<<"follow: id:"<<inFlags[surfaceFollow]->id<<" status: "<<qstr<<" contracted? "<<inFlags[surfaceFollow]->isContracted();
        hasFollow = true;
    }
    //subExecute contains all the actions for complaint coating and layering after leader election and returns the movement the particle should return
    Movement movement= subExecute();

    std::string status = "id: "+std::to_string(id);
    if(isContracted())
    {
        status+= " contracted: clean head,tail,expOut";
        for(int i =0; i<10;i++)
        {
            outFlags[i].status = "C";
        }
        switch(movement.type)
        {
        case MovementType::Expand:
            status+=" head = expOut of parent, tail = contr, update expOut";
            //put flags where they will be after expansion happens- separate channel from flags with current information about contracted particle
            for(auto it = headLabelsAfterExpansion(headMarkDirBefore).cbegin(); it != headLabelsAfterExpansion(headMarkDirBefore).cend(); ++it) {
                auto label = *it;
                outFlags[label].status += "H";

            }
            for(auto it = tailLabelsAfterExpansion(headMarkDirBefore).cbegin(); it != tailLabelsAfterExpansion(headMarkDirBefore
                                                                                                               ).cend(); ++it) {
                auto label = *it;
                outFlags[label].status += "T";

            }
            break;
        case MovementType::Idle:

            //safe to pass and receive tokens, etc
            if(hasParent && hasFollow && !hasNeighborInPhase(Phase::Inactive)&& hasNeighborInPhase(Phase::Static))//having a follower requires being on surface
            status += " execute LE";
            else
                status+=" no LE";
            break;
        }

        QString qstr(status.c_str());
        qDebug()<<qstr;
    }
    if(isExpanded())
    {

        status+=" expanded: clean contr,contrOut";
        switch(movement.type)
        {
        case MovementType::Contract:
            status +=" reg contract";
            if(tailOnSurface())
            {
                status+= "tail on surface";
            }
            else
            {
                status+=" tail not on surface";
            }
            status+=" contr=head, update contrOut";
            break;
        case MovementType::HandoverContract:
            status+=" contr=head, update contrOut";
            break;
        case MovementType::Idle:
            //head can execute because its between parent and tail
            if(tailOnSurface())
            {
            if(hasParent && !hasNeighborInPhase(Phase::Inactive)&& hasNeighborInPhase(Phase::Static))
            {
                status +=" head LE (toTail)";
            }
            //tail can execute because its between head and follow
            if(hasFollow && !hasNeighborInPhase(Phase::Inactive)&& hasNeighborInPhase(Phase::Static))
            {
                status +=" follow LE";
            }
            }
            else//tail off surface, head is maybe betweeen two particles on surface
            {
                if(hasParent && hasFollow)
                {
                    status += " head LE (parent/follow)";
                }
            }
            status+=", update expOut";
            break;
        }
        for(auto it = headLabels().cbegin(); it != headLabels().cend(); ++it) {
            auto label = *it;
            outFlags[label].status = "H";
        }
        for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
            auto label = *it;
            outFlags[label].status ="T";
        }
        QString qstr(status.c_str());
        qDebug()<<qstr;
    }

      //conditions for returning empty for termination of algorithm
    if(phase ==Phase::retiredLeader && !sentBorder && startedRetired && !hasNeighborInPhase(Phase::Lead) && !hasNeighborInPhase(Phase::Follow))
        return Movement(MovementType::Empty);

    qDebug()<<"movement: "<<(int)movement.type;
    return movement;
}
Movement UniversalCoating::subExecute()
{

    updateNeighborStages();

    if(phase == Phase::Lead || phase == Phase::retiredLeader)
        unsetFollowIndicator();

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
            bool tailOnSurface = false;
            for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
                auto label = *it;
                if(neighborIsInPhase(label,Phase::Static))
                    tailOnSurface = true;
            }

            if(!hasFollower  && tailOnSurface)
            {
                // qDebug()<<"no follow and tail on surface, not contracting";
                return Movement(MovementType::Idle);
            }
        }
        if(phase == Phase::Lead)
        {
            // qDebug()<<"expanded leader";
            if(phase == Phase::Lead && hasNeighborInPhase(Phase::Border))
            {
                borderPasses++;
            }

            if(hasNeighborInPhase(Phase::Inactive) || tailReceivesFollowIndicator()) {
                //  qDebug()<<"expanded leader: handover";

                return Movement(MovementType::HandoverContract, tailContractionLabel());
            } else {

                // qDebug()<<"expanded leader: contract";

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
            if(phase==Phase::Follow && hasNeighborInPhase(Phase::retiredLeader))
            {
                //  qDebug()<<"expanded follower on retired: followers <2, idling";

            }
            return Movement(MovementType::Idle);
        }

        setFollowIndicatorLabel(followDir);
        superLeader = isSuperLeader();//to know when contracted
       /* if(phase==Phase::Hold && hasNeighborInPhase(Phase::Static))
        {
            qDebug()<<"fix head for super leader";
            followDir = labelToDir(getSurfaceParentDir());
            headMarkDir = followDir;
            setFollowIndicatorLabel(followDir);

        }*/

        if(isSuperLeader() && phase == Phase::Hold)
           {
               setPhase(Phase::Normal);
           }

        if(hasNeighborInPhase(Phase::Inactive) || tailReceivesFollowIndicator()) {


            if(phase==Phase::Follow && hasNeighborInPhase(Phase::retiredLeader))
            {
                //  qDebug()<<"expanded follower on retired: handover contract";
                unsetFollowIndicator();
                setPhase(Phase::Lead);
                return Movement(MovementType::Idle);



            }
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        } else {


            return Movement(MovementType::Contract, tailContractionLabel());
        }
    }
    else
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

            setBlock();
            if(hasNeighborInPhase(Phase::Static) || hasNeighborInPhase(Phase::StaticBorder) )
            {
                //
                //handleElectionTokens();
                if(phase == Phase::Border)
                    return Movement(MovementType::Idle);
            }

            if((hasNeighborInPhase(Phase::retiredLeader) || hasNeighborInPhase(Phase::Border))&& phase!=Phase::Lead && phase!=Phase::retiredLeader) {
                //if(hasNeighborInPhase(Phase::Border) && !hasNeighborInPhase(Phase::retiredLeader))
                //    qDebug()<<"started on top of border";
                if(!(phase==Phase::Follow && headMarkDir!=-1 && inFlags[headMarkDir]!=nullptr &&
                     inFlags[headMarkDir]->isExpanded() && inFlags[headMarkDir]->phase==Phase::Lead))//follower behind expanded leader needs to complete handover 1st
                {
                    // qDebug()<<"on retired, set to lead";
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
                else
                {
                    // qDebug()<<"follower with expanded parent, wait to lead";
                }

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

                    auto surfaceFollower = firstNeighborInPhase(Phase::Static);
                    if(surfaceFollower!=-1)
                    {
                        while(neighborIsInPhase(surfaceFollower,Phase::Static))
                        {
                            surfaceFollower = (surfaceFollower+1)%6;
                        }

                    }

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
                    followDir = getSurfaceParentDir();
                    headMarkDir = followDir;

                    auto temp = dirToHeadLabelAfterExpansion(followDir, moveDir);
                    setFollowIndicatorLabel(temp);

                    if(inFlags[moveDir]==nullptr || inFlags[moveDir]->isExpanded())
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
                //  if(headMarkDir>-1 && inFlags[headMarkDir]!=nullptr)
                //    qDebug()<<"expanded parent? "<<inFlags[headMarkDir]->isExpanded();

                downDir= getDownDir();
                if(downDir == -1)
                {
                    //qDebug()<<"no down dir, going to follow";
                    setPhase(Phase::Follow);
                    return Movement(MovementType::Idle);
                }
                else
                    downDir=labelToDir(downDir);

                Lnumber = getLnumber();
                setLayerNumber(Lnumber);
                int moveDir = getMoveDir();

                if (neighborIsInPhase(moveDir, Phase::Border)|| neighborIsInPhaseandLayer(moveDir, Phase::retiredLeader, Lnumber) ||(hasNeighborInPhase(Phase::Border)))//??aya checke Lnumber nemikhad? ghanunan na age movedir dorost amal karde bashe
                {

                    setPhase(Phase::retiredLeader);
                    getLeftDir();
                    Q_ASSERT(leftDir>=0 && leftDir<=5);
                    NumFinishedNeighbors = CountFinishedSides(leftDir, rightDir);
                    // qDebug()<<"num fin neighbors: "<<NumFinishedNeighbors;
                    setNumFinishedNeighbors(NumFinishedNeighbors);
                    headMarkDir = downDir;//-1;

                    setPhase(Phase::retiredLeader);

                    return Movement(MovementType::Idle);
                }
                else
                {
                    for(int label= 0; label<10;label++)
                    {

                        if(inFlags[label]!=nullptr && inFlags[label]->buildBorder)
                        {

                            headMarkDir = label;
                            setPhase(Phase::retiredLeader);
                            int borderBuildDir = (headMarkDir+3)%6;
                            //  while(neighborIsInPhase(borderBuildDir,Phase::Static) || neighborIsInPhase(borderBuildDir,Phase::StaticBorder))
                            //     borderBuildDir = (borderBuildDir+1)%6;
                            // outFlags[borderBuildDir].buildBorder= true;

                            Lnumber = getLnumber();
                            setLayerNumber(Lnumber);
                            downDir= getDownDir();

                            // qDebug()<<"trying to border? l 2"<<Lnumber;
                            setPhase(Phase::retiredLeader);
                            return Movement(MovementType::Idle);
                        }
                    }
                }

                setContractDir(moveDir);
                headMarkDir = downDir;
                setPhase(Phase::Lead);

                qDebug()<<"lead expand";

                return Movement(MovementType::Expand, moveDir);

            }
            else if(phase == Phase::Follow) {

                if(inFlags[followDir]==nullptr)//with bordering, this loss of leader can happen
                {
                    //  qDebug()<<"null followdir";
                    setPhase(Phase::Normal);
                    return Movement(MovementType::Idle);
                }
                //Q_ASSERT(inFlags[followDir] != nullptr);
                if(hasNeighborInPhase(Phase::Static) || hasNeighborInPhase(Phase::StaticBorder)) {
                    //  qDebug()<<"static or static border, to lead";

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
                    // qDebug()<<"parent expand, trying to expand";
                    int expansionDir = followDir;
                    setContractDir(expansionDir);
                    followDir = updatedFollowDir();
                    headMarkDir = followDir;
                    auto temp = dirToHeadLabelAfterExpansion(followDir, expansionDir);
                    Q_ASSERT(labelToDirAfterExpansion(temp, expansionDir) == followDir);
                    setFollowIndicatorLabel(temp);
                    //  qDebug()<<"follow expanding";
                    qDebug()<<"follow expand";
                    return Movement(MovementType::Expand, expansionDir);
                }
                if(hasNeighborInPhase(Phase::retiredLeader)) {

                    //  qDebug()<<"Follow with retiredlead neighbor"<<id;
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

                return Movement(MovementType::Idle);

            }

            else if(phase == Phase::retiredLeader )
            {
                //  qDebug()<<"retired leader execute";
                getLeftDir();
                Q_ASSERT(leftDir >=0 && leftDir<=5);
                NumFinishedNeighbors = CountFinishedSides(leftDir, rightDir);
                setNumFinishedNeighbors(NumFinishedNeighbors);
                if(NumFinishedNeighbors == 2 )
                {
                    for(int label= 0; label<10;label++)
                    {
                        if(inFlags[label]!=nullptr && inFlags[label]->buildBorder)
                        {
                            //  qDebug()<<"retiredLeader finished, sending border";
                            headMarkDir = label;
                            int borderBuildDir = (headMarkDir+3)%6;
                            while(neighborIsInPhase(borderBuildDir,Phase::Static) || neighborIsInPhase(borderBuildDir,Phase::StaticBorder))
                                borderBuildDir = (borderBuildDir+1)%6;

                            if(outFlags[borderBuildDir].buildBorder ==false)
                            {
                                sentBorder = true;
                                outFlags[borderBuildDir].buildBorder= true;
                            }
                            //  qDebug()<<"trying to border? b";
                            return Movement(MovementType::Idle);
                        }
                    }

                }
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
                //    qDebug()<<(int)phase<<" to retired? ";
                setPhase(Phase::retiredLeader);

                Lnumber = getLnumber();
                setLayerNumber(Lnumber);
                setPhase(Phase::retiredLeader);
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
                        qDebug()<<"motionblock expand";
                        return Movement(MovementType::Expand, expansionDir);
                    }
                }



                return Movement(MovementType::Idle);
            }
        }
    }
    //qDebug()<<"empty: "<<(int)phase;

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
    if(phase==Phase::Static)
        return true;
    return false;
}

bool UniversalCoating::isSemiActive() const
{
    if(phase == Phase::retiredLeader)
        return true;
    return false;
}

void UniversalCoating::setPhase(const Phase _phase)
{
    phase = _phase;
    for(int label = 0; label < 10; label++) {
        outFlags[label].phase = phase;
    }
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
        bool inBorder = false;
        bool outBorder = false;
        for(int i =0; i<6; i++)
        {
            if(inFlags[i]!=nullptr && inFlags[i]->buildBorder)
                inBorder = true;
            if(outFlags[i].buildBorder)
                outBorder = true;
        }
        if(inBorder && outBorder)
        {

            headMarkColor = 0x666666;
            tailMarkColor = 0x666666
                    ;
        }
        else if(inBorder)
        {
            headMarkColor = 0xD3D3D3;
            tailMarkColor = 0xD3D3D3;
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
   /* if(phase == Phase::Normal || phase == Phase::Wait )
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
    }*/
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
            while (neighborIsInPhase(label, Phase::Static)  || neighborIsInPhase(label,Phase::StaticBorder)//  || neighborIsInPhase(label,Phase::Border)
                   ){
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
                // qDebug()<<"counting ret finished: "<<countRetiredareFinished(0)<<" "<<countRetiredareFinished(1);
                if(countRetiredareFinished(0) == ZeroLayerGreens)
                {
                    label= firstNeighborInPhaseandLayer(Phase::retiredLeader, 0);
                }
                else if(countRetiredareFinished(1) == OneLayerGreens)
                {
                    label = firstNeighborInPhaseandLayer(Phase::retiredLeader, 1);
                }
                /*  else if(ZeroLayerGreens == 2 && OneLayerGreens == 1)
                {
                    qDebug()<<"fixit 2 zeros 1 one";
                    label = firstNeighborInPhaseandLayer(Phase::retiredLeader,1);
                }
                else if (OneLayerGreens == 2  && ZeroLayerGreens ==1 )
                {
                    qDebug()<<"fixit 2 ones 1 zero";

                    label = firstNeighborInPhaseandLayer(Phase::retiredLeader,0);
                }*/
                else
                {
                    label= -1;
                }
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
        if(inFlags[downDir]->phase == Phase::Static || inFlags[downDir]->phase == Phase::StaticBorder
                )
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

LocData UniversalCoating::handlePositionElection(LocData myData, LocData followData, LocData parentData)
{
    return ExecuteLeaderElection(myData,
                                 followData.forwardTokens,followData.electionRole,followData.electionSubphase,
                                 parentData.backTokens,parentData.electionRole,parentData.electionSubphase);
}

LocData UniversalCoating::ExecuteLeaderElection(LocData myData,std::array<Token, 15> followTokens,ElectionRole followRole,ElectionSubphase followSubphase,
                                                std::array<Token, 15>parentTokens,ElectionRole parentRole,ElectionSubphase parentSubphase)

{
    self = myData;
    this->followTokens = followTokens;
    this->parentTokens = parentTokens;
    //  qDebug()<<"   execute: me: "<<(int)myData.electionRole<<" parent: "<<(int)parentRole<<" follow: "<<(int)followRole;
    // qDebug()<<"   subphase: "<<(int)self.electionSubphase;
    tokenCleanup(); // clean token remnants before doing new actions
    bool nextAgentDir = false;
    bool prevAgentDir = true;
    tokenCleanup(); // clean token remnants before doing new actions

    if(self.electionRole == ElectionRole::Candidate) {
        // this first block of tasks should be performed by any candidate, regardless of subphase
        // if there is an incoming active segment cleaning token, consume it
        if(peekAtToken(TokenType::ActiveSegmentClean, nextAgentDir) != -1) {
            performActiveClean(2); // clean the front side only
            receiveToken(TokenType::ActiveSegmentClean, nextAgentDir);
        }
        // if there is an incoming passive segment cleaning token, consume it
        if(peekAtToken(TokenType::PassiveSegmentClean, prevAgentDir) != -1) {
            performPassiveClean(1); // clean the back side only
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
            if( self.switches.at((int)SwitchVariable::absorbedActiveToken)==0) {
                if(receiveToken(TokenType::ActiveSegment, nextAgentDir).value == 2) { // absorbing the last active token
                    sendToken(TokenType::FinalSegmentClean, nextAgentDir, 2);
                } else { // this candidate is now covered
                    Q_ASSERT(canSendToken(TokenType::ActiveSegmentClean, prevAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir));
                    sendToken(TokenType::PassiveSegmentClean, nextAgentDir, 1);
                    performPassiveClean(2); // clean the front side only
                    paintFrontSegment(QColor("dimgrey").rgb());
                    sendToken(TokenType::ActiveSegmentClean, prevAgentDir, 1);
                    performActiveClean(1); // clean the back side only
                    self.switches.at((int)SwitchVariable::absorbedActiveToken) = 1;
                    self.switches.at((int)SwitchVariable:: isCoveredCandidate) = 1;
                    setElectionRole(ElectionRole::Demoted);
                    return self; // completed subphase and thus shouldn't perform any more operations in this round
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
            if(self.switches.at((int)SwitchVariable::waitingForTransferAck)==1) {
                self.switches.at((int)SwitchVariable::gotAnnounceBeforeAck) = 1;
            } else if(self.switches.at((int)SwitchVariable::comparingSegment)==1) {
                self.switches.at((int)SwitchVariable::gotAnnounceInCompare) = 1;
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
        // the next block of tasks are dependent upon subphase
        if(self.electionSubphase == ElectionSubphase::SegmentComparison) {
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
                if(finalCleanValue == 0 && self.switches.at((int)SwitchVariable::gotAnnounceInCompare)==0) { // if this candidate did not cover any tokens and was not transferred candidacy, demote
                    setElectionRole(ElectionRole::Demoted);
                } else {
                    setElectionSubphase(ElectionSubphase::CoinFlip);
                }
                self.switches.at((int)SwitchVariable::comparingSegment) = 0;
                self.switches.at((int)SwitchVariable::gotAnnounceInCompare) = 0;
                return self; // completed subphase and thus shouldn't perform any more operations in this round
            } else if(self.switches.at((int)SwitchVariable::comparingSegment)==0) {
                // begin segment comparison by generating a segment lead token
                Q_ASSERT(canSendToken(TokenType::SegmentLead, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir));
                sendToken(TokenType::SegmentLead, nextAgentDir, 1);
                paintFrontSegment(QColor("red").rgb());
                self.switches.at((int)SwitchVariable::comparingSegment) = 1;
            }
        } else if(self.electionSubphase == ElectionSubphase::CoinFlip) {
            if(peekAtToken(TokenType::CandidacyAck, nextAgentDir) != -1) {
                // if there is an acknowledgement waiting, consume the acknowledgement and proceed to the next subphase
                receiveToken(TokenType::CandidacyAck, nextAgentDir);
                paintFrontSegment(QColor("dimgrey").rgb());
                setElectionSubphase(ElectionSubphase::SolitudeVerification);
                if(self.switches.at((int)SwitchVariable::gotAnnounceBeforeAck)==0) {
                    setElectionRole(ElectionRole::Demoted);
                }
                self.switches.at((int)SwitchVariable::waitingForTransferAck) = 0;
                self.switches.at((int)SwitchVariable::gotAnnounceBeforeAck) = 0;
                return self; // completed subphase and thus shouldn't perform any more operations in this round
            } else if(self.switches.at((int)SwitchVariable::waitingForTransferAck)==0 && randBool()) {
                // if I am not waiting for an acknowlegdement of my previous announcement and I win the coin flip, announce a transfer of candidacy
                Q_ASSERT(canSendToken(TokenType::CandidacyAnnounce, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)); // there shouldn't be a call to make two announcements
                sendToken(TokenType::CandidacyAnnounce, nextAgentDir, 1);
                paintFrontSegment(QColor("orange").rgb());
                self.switches.at((int)SwitchVariable::waitingForTransferAck) = 1;
            }
        } else if(self.electionSubphase == ElectionSubphase::SolitudeVerification) {
            // if the agent needs to, generate a lane 1 vector token
            if(self.switches.at((int)SwitchVariable::generateVectorDir) != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                sendToken(TokenType::SolitudeVectorL1, nextAgentDir, self.switches.at((int)SwitchVariable::generateVectorDir));
                self.switches.at((int)SwitchVariable::generateVectorDir) = -1;
            }

            if(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != -1) {
                // consume all vector tokens that have not been matched, decide that solitude has failed
                Q_ASSERT(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != 0); // matched tokens should have dropped
                receiveToken(TokenType::SolitudeVectorL2, nextAgentDir);
                self.switches.at((int)SwitchVariable::sawUnmatchedToken) = 1;
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
                    if(self.switches.at((int)SwitchVariable::sawUnmatchedToken)==0 && (leadValue / 1000) == 1) { // if it did not consume an unmatched token and it assures it's matching with itself, go to inner/outer test
                        setElectionRole(ElectionRole::SoleCandidate);
                    } else { // if solitude verification failed, then do another coin flip compeititon
                        setElectionSubphase(ElectionSubphase::SegmentComparison);
                    }
                    self.switches.at((int)SwitchVariable::createdLead )= 0;
                    self.switches.at((int)SwitchVariable::sawUnmatchedToken)= 0;
                    return self; // completed subphase and thus shouldn't perform any more operations in this round
                }
            } else if(self.switches.at((int)SwitchVariable::createdLead)==0) {
                // to begin the solitude verification, create a lead token with an orientation to communicate to its segment
                Q_ASSERT(canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) &&
                         canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)); // there shouldn't be a call to make two leads
                sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 100 + encodeVector(std::make_pair(1,0))); // lap 1 in direction (1,0)
                paintFrontSegment(QColor("deepskyblue").darker().rgb());
                self.switches.at((int)SwitchVariable::createdLead )= 1;
                self.switches.at((int)SwitchVariable::generateVectorDir) = encodeVector(std::make_pair(1,0));
            }
        }
    } else if(self.electionRole == ElectionRole::SoleCandidate) {
        if(self.switches.at((int)SwitchVariable::testingBorder)==0) { // begin the inner/outer border test
            Q_ASSERT(canSendToken(TokenType::BorderTest, nextAgentDir));
            //sendToken(TokenType::BorderTest, nextAgentDir, addNextBorder(0));
            paintFrontSegment(-1);
            self.switches.at((int)SwitchVariable:: testingBorder) = 1;
        } else if(peekAtToken(TokenType::BorderTest, prevAgentDir) != -1) { // test is complete
            int borderSum = receiveToken(TokenType::BorderTest, prevAgentDir).value;
            paintBackSegment(-1);
            if(borderSum == 1) { // outer border, agent becomes the leader
                setElectionRole(ElectionRole::Leader);
            } else if(borderSum == 4) { // inner border, demote agent and set to finished
                //setElectionRole(ElectionRole::Finished);
            }
            self.switches.at((int)SwitchVariable::testingBorder) = 0;
            return self; // completed subphase and thus shouldn't perform any more operations in this round
        }
    } else if(self.electionRole == ElectionRole::Demoted) {
        // SUBPHASE: Segment Comparison
        // pass passive segment cleaning tokens forward, and perform cleaning
        if(peekAtToken(TokenType::PassiveSegmentClean, prevAgentDir) != -1 && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
            performPassiveClean(3); // clean the full segment
            sendToken(TokenType::PassiveSegmentClean, nextAgentDir, receiveToken(TokenType::PassiveSegmentClean, prevAgentDir).value);
            paintFrontSegment(QColor("dimgrey").rgb()); paintBackSegment(QColor("dimgrey").rgb());
        }
        // pass active segment cleaning tokens backward, and perform cleaning
        if(peekAtToken(TokenType::ActiveSegmentClean, nextAgentDir) != -1 && canSendToken(TokenType::ActiveSegmentClean, prevAgentDir)) {
            performActiveClean(3); // clean the full segment
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
            if(self.switches.at((int)SwitchVariable::absorbedActiveToken)==0) {
                if(receiveToken(TokenType::ActiveSegment, nextAgentDir).value == 2) { // if absorbing the final active token, generate the final cleaning token
                    sendToken(TokenType::FinalSegmentClean, nextAgentDir, 0); // the final segment cleaning token begins as having not seen covered candidates
                } else {
                    self.switches.at((int)SwitchVariable::absorbedActiveToken) = 1;
                }
            } else if(canSendToken(TokenType::ActiveSegment, prevAgentDir) && canSendToken(TokenType::ActiveSegmentClean, prevAgentDir)) {
                // pass active token backward if doing so does not pass the active cleaning token
                sendToken(TokenType::ActiveSegment, prevAgentDir, receiveToken(TokenType::ActiveSegment, nextAgentDir).value);
            }
        }
        // pass final cleaning token forward, perform cleaning, and check for covered candidates
        if(peekAtToken(TokenType::FinalSegmentClean, prevAgentDir) != -1 && canSendToken(TokenType::FinalSegmentClean, nextAgentDir)) {
            int finalCleanValue = receiveToken(TokenType::FinalSegmentClean, prevAgentDir).value;
            if(finalCleanValue != 2 && self.switches.at((int)SwitchVariable::isCoveredCandidate)==1) {
                finalCleanValue = 1;
            }
            self.switches.at((int)SwitchVariable:: absorbedActiveToken) = 0;
            self.switches.at((int)SwitchVariable::isCoveredCandidate) = 0;
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
                self.switches.at((int)SwitchVariable::generateVectorDir) = encodeVector(augmentDirVector(leadVector, offset));
                sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 100 + self.switches.at((int)SwitchVariable::generateVectorDir)); // lap 1 + new encoded direction vector
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
        if(self.switches.at((int)SwitchVariable::generateVectorDir) != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
            sendToken(TokenType::SolitudeVectorL1, nextAgentDir, self.switches.at((int)SwitchVariable::generateVectorDir));
            self.switches.at((int)SwitchVariable::generateVectorDir) = -1;
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
            // int borderSum = addNextBorder(receiveToken(TokenType::BorderTest, prevAgentDir).value);
            sendToken(TokenType::BorderTest, nextAgentDir, 0);
            paintFrontSegment(-1); paintBackSegment(-1);
            //  setElectionRole(E::Finished);
        }
    }
    return self;
}

void UniversalCoating::paintFrontSegment(const int color)
{
}

void UniversalCoating::paintBackSegment(const int color)
{
}

bool UniversalCoating::canSendToken(TokenType type, bool toBack) const
{
    if(toBack)
        return (self.backTokens.at((int) type).value == -1 && !followTokens.at((int) type).receivedToken);
    else
        return (self.forwardTokens.at((int) type).value == -1 && !parentTokens.at((int) type).receivedToken);

}

void UniversalCoating::sendToken(TokenType type, bool toBack, int value)
{
    // qDebug()<<"send: "<<(int)type<<" value: "<<value<<" back? "<<(int)toBack;
    Q_ASSERT(canSendToken(type, toBack));
    if(toBack)
        self.backTokens.at((int) type).value = value;
    else
        self.forwardTokens.at((int) type).value = value;

}

int UniversalCoating::peekAtToken(TokenType type, bool fromBack) const
{
    if(fromBack)
    {
        if(self.backTokens.at((int) type).receivedToken) {
            // if this agent has already read this token, don't peek the same value again
            return -1;
        } else {
            return followTokens.at((int) type).value;
        }
    }
    else//from front
    {
        if(self.forwardTokens.at((int) type).receivedToken) {
            // if this agent has already read this token, don't peek the same value again
            return -1;
        } else {
            return parentTokens.at((int) type).value;
        }
    }
}

Token UniversalCoating::receiveToken(TokenType type, bool fromBack)
{
    Q_ASSERT(peekAtToken(type, fromBack) != -1);

    if(fromBack)
    {
        self.backTokens.at((int) type).receivedToken = true;
        //  qDebug()<<"received: "<<(int)type<<" value: "<<followTokens.at((int)type).value;
        return followTokens.at((int) type);
    }
    else//from front
    {
        self.forwardTokens.at((int) type).receivedToken = true;
        //  qDebug()<<"received: "<<(int)type<<" value: "<<parentTokens.at((int)type).value;
        return parentTokens.at((int) type
                               );
    }

}

void UniversalCoating::tokenCleanup()
{
    for(auto tokenType : {TokenType::SegmentLead, TokenType::PassiveSegmentClean, TokenType::FinalSegmentClean,
        TokenType::CandidacyAnnounce, TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1, TokenType::BorderTest}) {
        if(parentTokens.at((int) tokenType).receivedToken) {
            self.forwardTokens.at((int) tokenType).value = -1;
        }
        if(followTokens.at((int) tokenType).value == -1) {
            self.backTokens.at((int) tokenType).receivedToken = false;
        }
    }
    for(auto tokenType : {TokenType::PassiveSegment, TokenType::ActiveSegment, TokenType::ActiveSegmentClean, TokenType::CandidacyAck,
        TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
        if(followTokens.at((int) tokenType).receivedToken) {
            self.backTokens.at((int) tokenType).value = -1;
        }
        if(parentTokens.at((int) tokenType).value == -1) {
            self.forwardTokens.at((int) tokenType).receivedToken = false;
        }
    }
}

void UniversalCoating::performPassiveClean(const int region)
{
    Q_ASSERT(1 <= region && region <= 3); // 1 => back, 2 => front, 3 => both
    if(region != 2) { // back
        for(auto tokenType : {TokenType::SegmentLead, TokenType::CandidacyAnnounce, TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1}) {
            if(peekAtToken(tokenType, true) != -1) {
                receiveToken(tokenType, true);
            }
        }
    }
    if(region != 1) { // front
        for(auto tokenType : {TokenType::PassiveSegment, TokenType::CandidacyAck, TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
            if(peekAtToken(tokenType, false) != -1) {
                receiveToken(tokenType, false);
            }
        }
    }
}

void UniversalCoating::performActiveClean(const int region)
{
    Q_ASSERT(1 <= region && region <= 3); // 1 => back, 2 => front, 3 => both
    if(region != 2 && peekAtToken(TokenType::FinalSegmentClean, true) != -1) { // back
        receiveToken(TokenType::FinalSegmentClean, true);
    }
    if(region != 1 && peekAtToken(TokenType::ActiveSegment, false) != -1) { // front
        receiveToken(TokenType::ActiveSegment, false);
    }
    self.switches.at((int)SwitchVariable::absorbedActiveToken) = 0;//0 = false
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
    self.electionRole = role;
}

void UniversalCoating::setElectionSubphase(ElectionSubphase subphase)
{
    self.electionSubphase = subphase;
}





void UniversalCoating::printTokens(LocData ldata)
{
    qDebug()<<"forward out: ";
    for(auto tokenType : {TokenType::SegmentLead, TokenType::PassiveSegmentClean, TokenType::FinalSegmentClean,
        TokenType::CandidacyAnnounce, TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1, TokenType::BorderTest, TokenType::PassiveSegment, TokenType::ActiveSegment, TokenType::ActiveSegmentClean, TokenType::CandidacyAck,
        TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
        if(ldata.forwardTokens.at((int) tokenType).receivedToken) {
            qDebug()<<"  token "<<(int)tokenType<<" received";
        }
        if(ldata.forwardTokens.at((int) tokenType).value != -1) {
            qDebug()<<"  token "<<(int)tokenType<<" value: "<<ldata.forwardTokens.at((int) tokenType).value;
        }
    }
    qDebug()<<"backward out: ";
    for(auto tokenType : {TokenType::SegmentLead, TokenType::PassiveSegmentClean, TokenType::FinalSegmentClean,
        TokenType::CandidacyAnnounce, TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1, TokenType::BorderTest, TokenType::PassiveSegment, TokenType::ActiveSegment, TokenType::ActiveSegmentClean, TokenType::CandidacyAck,
        TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
        if(ldata.backTokens.at((int) tokenType).receivedToken) {
            qDebug()<<"  token "<<(int)tokenType<<" received";
        }
        if(ldata.backTokens.at((int) tokenType).value != -1) {
            qDebug()<<"  token "<<(int)tokenType<<" value: "<<ldata.backTokens.at((int) tokenType).value;
        }
    }



}
void UniversalCoating::cleanHeadLocData()
{
    int typenum = 0;
    for(auto token = headLocData.backTokens.begin(); token != headLocData.backTokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        ++typenum;
    }

    typenum = 0;
    for(auto token = headLocData.forwardTokens.begin(); token != headLocData.forwardTokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        ++typenum;
    }

    headLocData.electionRole = ElectionRole::Demoted;

    headLocData.electionSubphase = ElectionSubphase::SegmentComparison;



    for (int i =0; i<headLocData.switches.size();i++)
    {
        headLocData.switches.at(i) = 0;
    }
    headLocData.switches.at((int)SwitchVariable::generateVectorDir)=-1;

}
void UniversalCoating::cleanTailLocData()
{
    int typenum = 0;
    for(auto token = tailLocData.backTokens.begin(); token != tailLocData.backTokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        ++typenum;
    }

    typenum = 0;
    for(auto token = tailLocData.forwardTokens.begin(); token != tailLocData.forwardTokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        ++typenum;
    }

    tailLocData.electionRole = ElectionRole::Demoted;

    tailLocData.electionSubphase = ElectionSubphase::SegmentComparison;


    for (int i =0; i<tailLocData.switches.size();i++)
    {
        tailLocData.switches.at(i) = 0;
    }
    tailLocData.switches.at((int)SwitchVariable::generateVectorDir)=-1;

}
bool UniversalCoating::expandedOnSurface()
{
    bool tailOnSurface = false;
    bool headOnSurface = false;
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        auto label = *it;
        if(neighborIsInPhase(label,Phase::Static))
            tailOnSurface = true;
    }
    for(auto it = headLabels().cbegin(); it != headLabels().cend(); ++it) {
        auto label = *it;
        if(neighborIsInPhase(label,Phase::Static))
            headOnSurface = true;
    }
    return tailOnSurface && headOnSurface;
}
bool UniversalCoating::tailOnSurface()
{
    bool tailOnSurface = false;
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        auto label = *it;
        if(neighborIsInPhase(label,Phase::Static))
            tailOnSurface = true;
    }

    return tailOnSurface ;
}
int UniversalCoating::getSurfaceFollowDir()
{
    auto surfaceFollower = firstNeighborInPhase(Phase::Static);
    if(surfaceFollower!=-1)
    {
        if(isExpanded())
        {
            while(neighborIsInPhase(surfaceFollower,Phase::Static))
            {
                surfaceFollower = (surfaceFollower+1)%10;
            }
        }
        else
        {
            while(neighborIsInPhase(surfaceFollower,Phase::Static))
            {
                surfaceFollower = (surfaceFollower+1)%6;
            }
        }

    }
    return surfaceFollower;
}
int UniversalCoating::getSurfaceParentDir()
{
    auto surfaceParent = firstNeighborInPhase(Phase::Static);
    if(surfaceParent!=-1)
    {
        if(isExpanded())
        {
            while(neighborIsInPhase(surfaceParent,Phase::Static))
            {
                surfaceParent = (surfaceParent+9)%10;
            }
        }
        else
        {
            while(neighborIsInPhase(surfaceParent,Phase::Static))
            {
                surfaceParent = (surfaceParent+5)%6;
            }
        }

    }
    return surfaceParent;
}
}
