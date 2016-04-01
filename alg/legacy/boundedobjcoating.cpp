#include <algorithm>
#include <cmath>
#include <deque>
#include <set>

#include "alg/legacy/boundedobjcoating.h"
#include "alg/legacy/legacyparticle.h"
#include "alg/legacy/legacysystem.h"

namespace BoundedObjCoating
{
BoundedObjCoatingFlag::BoundedObjCoatingFlag()
    : followIndicator(false),
      Lnumber(-1),
      NumFinishedNeighbors(0)
{
}

BoundedObjCoatingFlag::BoundedObjCoatingFlag(const BoundedObjCoatingFlag& other)
    : Flag(other),
      phase(other.phase),
      contractDir(other.contractDir),
      followIndicator(other.followIndicator),
      Lnumber(other.Lnumber),
      NumFinishedNeighbors(other.NumFinishedNeighbors)
{
}

BoundedObjCoating::BoundedObjCoating(const Phase _phase)
{
    setPhase(_phase);
    downDir = -1;
    Lnumber = -1;
    leftDir = -1;
    rightDir = -1;
    NumFinishedNeighbors = 0;
}

BoundedObjCoating::BoundedObjCoating(const BoundedObjCoating& other)
    : AlgorithmWithFlags(other),
      phase(other.phase),
      followDir(other.followDir),
      Lnumber(other.Lnumber),
      downDir(other.downDir),
      leftDir(other.leftDir),
      rightDir(other.rightDir),
      NumFinishedNeighbors(other.NumFinishedNeighbors)
{
}

BoundedObjCoating::~BoundedObjCoating()
{
}

std::shared_ptr<LegacySystem> BoundedObjCoating::instance(const int numStaticParticles, const int numParticles, const float holeProb)
{
    std::shared_ptr<LegacySystem> system = std::make_shared<LegacySystem>();

    std::deque<Node> orderedSurface;
    std::set<Node> occupied;

    Node pos;
    Node Start;
    Node End;
    int lastOffset = 0;
    while((int) system->size() < numStaticParticles) {
        system->insertParticle(LegacyParticle(std::make_shared<BoundedObjCoating>(Phase::Static), randDir(), pos));
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

    while(counter < yMax) {
        system->insertParticle(LegacyParticle(std::make_shared<BoundedObjCoating>(Phase::Border), randDir(), pos1));
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
    }

    counter =0;
    while(counter < yMax) {
        system->insertParticle(LegacyParticle(std::make_shared<BoundedObjCoating>(Phase::Border), randDir(), pos2));
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
    while(numNonStaticParticles < numParticles) {
        if(candidates.empty()) {
            return system;
        }

        std::set<Node> nextCandidates;
        for(auto it = candidates.begin(); it != candidates.end() && numNonStaticParticles < numParticles; ++it) {
            if(randBool(1.0f - holeProb)) {
                system->insertParticle(LegacyParticle(std::make_shared<BoundedObjCoating>(Phase::Inactive), randDir(), *it));
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
        }
        nextCandidates.swap(candidates);
    }

    return system;
}

Movement BoundedObjCoating::execute()
{
    if(isExpanded()) {
        if(phase == Phase::Follow) {
            setFollowIndicatorLabel(followDir);
        }

        if(hasNeighborInPhase(Phase::Inactive) || tailReceivesFollowIndicator()) {
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        } else {
            return Movement(MovementType::Contract, tailContractionLabel());
        }
    } else { //particle is contracted

        if(phase == Phase::Inactive) {
            if(hasNeighborInPhase(Phase::Static)) {
                setPhase(Phase::Lead);

                downDir= getDownDir();


                //Q_ASSERT(downDir!=-1);
                if(downDir == -1)
                    return Movement(MovementType::Idle);
                else downDir=labelToDir(downDir);

                Lnumber = getLnumber();
                setLayerNumber(Lnumber);

                setPhase(Phase::Lead);

                return Movement(MovementType::Idle);
            }
            else if(hasNeighborInPhase(Phase::retiredLeader)){
                setPhase(Phase::Lead);
                downDir= getDownDir();


                //Q_ASSERT(downDir!=-1);
                if(downDir == -1)
                    return Movement(MovementType::Idle);
                else downDir=labelToDir(downDir);

                Lnumber = getLnumber();
                setLayerNumber(Lnumber);

                setPhase(Phase::Lead);

                return Movement(MovementType::Idle);
            }
            auto label = std::max(firstNeighborInPhase(Phase::Follow), firstNeighborInPhase(Phase::Lead));
            if(label != -1) {
                setPhase(Phase::Follow);
                followDir = labelToDir(label);
                setFollowIndicatorLabel(followDir);
                headMarkDir = followDir;

                setPhase(Phase::Follow);
                return Movement(MovementType::Idle);
            }
        } else if(phase == Phase::Follow) {
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
        } else if(phase == Phase::Lead) {

            downDir= getDownDir();

            if(downDir == -1)
                return Movement(MovementType::Idle);
            else downDir=labelToDir(downDir);

            Lnumber = getLnumber();
            setLayerNumber(Lnumber);
            int moveDir = getMoveDir();

            if (neighborIsInPhase(moveDir, Phase::Border) || neighborIsInPhaseandLayer(moveDir, Phase::retiredLeader, Lnumber))//??aya checke Lnumber nemikhad? ghanunan na age movedir dorost amal karde bashe
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
            else{

                setContractDir(moveDir);
                headMarkDir = downDir;
                setPhase(Phase::Lead);
                return Movement(MovementType::Expand, moveDir);
            }
        }else if(phase == Phase::retiredLeader )
        {

                getLeftDir();
                Q_ASSERT(leftDir >=0 && leftDir<=5);
                NumFinishedNeighbors = CountFinishedSides(leftDir, rightDir);
                setNumFinishedNeighbors(NumFinishedNeighbors);
                Q_ASSERT(NumFinishedNeighbors>=0 && NumFinishedNeighbors<=2);
                setPhase(Phase::retiredLeader);

                return Movement(MovementType::Idle);

       }
        return Movement(MovementType::Empty);
    }
}

std::shared_ptr<Algorithm> BoundedObjCoating::blank() const
{
    return std::make_shared<BoundedObjCoating>(Phase::Inactive);
}

std::shared_ptr<Algorithm> BoundedObjCoating::clone()
{
    return std::make_shared<BoundedObjCoating>(*this);
}

bool BoundedObjCoating::isDeterministic() const
{
    return true;
}

bool BoundedObjCoating::isStatic() const
{
    return phase == Phase::Static;
}
bool BoundedObjCoating::isRetired() const
{
    return phase == Phase::retiredLeader;
}


void BoundedObjCoating::setPhase(const Phase _phase)
{
    phase = _phase;

    if(phase == Phase::Lead) {

        headMarkColor = 0xff0000;
        tailMarkColor = 0xff0000;

    } else if(phase == Phase::Follow) {
        headMarkColor = 0x0000ff;
        tailMarkColor = 0x0000ff;
    } else if(phase == Phase::retiredLeader){
        if(NumFinishedNeighbors ==2)
        {
        headMarkColor = 0x00ff00;
        tailMarkColor = 0x00ff00;
        }
       /* else
        {
            headMarkColor = 0x00ffff;
            tailMarkColor = 0x00ffff;
        }*/
    } else if(phase == Phase::Inactive) {
        headMarkColor = -1;
        tailMarkColor = -1;
    } else  if(phase == Phase::Border)
    {
        headMarkColor = 0xff0000;
        tailMarkColor = 0xff0000;

    }else

    { // phase == Phase::Static
        headMarkColor = 0x000000;
        tailMarkColor = 0x000000;
    }

    for(int label = 0; label < 10; label++) {
        outFlags[label].phase = phase;
    }
}

bool BoundedObjCoating::neighborIsInPhase(const int label, const Phase _phase) const
{
    Q_ASSERT(0 <= label && label <= 9);
    return (inFlags[label] != nullptr && inFlags[label]->phase == _phase);
}

bool BoundedObjCoating::neighborIsInPhaseandLayer(const int label, const Phase _phase, const int L) const
{
    Q_ASSERT(isContracted());

    return (inFlags[label] != nullptr && inFlags[label]->phase == _phase && inFlags[label]->Lnumber == L);
}

int BoundedObjCoating::firstNeighborInPhase(const Phase _phase) const
{
    for(int label = 0; label < 10; label++) {
        if(neighborIsInPhase(label, _phase)) {
            return label;
        }
    }
    return -1;
}

int BoundedObjCoating::firstNeighborInPhaseandLayer(const Phase _phase, const int L) const
{
    for(int label = 0; label < 10; label++) {
        if(neighborIsInPhaseandLayer(label, _phase, L)){
            return label;
        }
    }
    return -1;
}
bool BoundedObjCoating::hasNeighborInPhase(const Phase _phase) const
{
    return (firstNeighborInPhase(_phase) != -1);
}

void BoundedObjCoating::setContractDir(const int contractDir)
{
    for(int label = 0; label < 10; label++) {
        outFlags[label].contractDir = contractDir;
    }
}

int BoundedObjCoating::updatedFollowDir() const
{
    int contractDir = inFlags[followDir]->contractDir;
    int offset = (followDir - inFlags[followDir]->dir + 9) % 6;
    int tempFollowDir = (contractDir + offset) % 6;
    Q_ASSERT(0 <= tempFollowDir && tempFollowDir <= 5);
    return tempFollowDir;
}

void BoundedObjCoating::unsetFollowIndicator()
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = false;
    }
}

void BoundedObjCoating::setFollowIndicatorLabel(const int label)
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = (i == label);
    }
}

bool BoundedObjCoating::tailReceivesFollowIndicator() const
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

int BoundedObjCoating::getMoveDir() const
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

int BoundedObjCoating::getDownDir() const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(phase==Phase::Lead);

    int label= -1;//??
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

int BoundedObjCoating::getLnumber() const
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

    return 0; // RG: previously, the function could reach this point without returning anything which caused a compiler warning
}

void BoundedObjCoating::getLeftDir()
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

int BoundedObjCoating::countGreenNeighbors(const Phase _phase, const int L) const
{
    int count=0;
    for(int dir = 0; dir < 10; dir++) {
         if(neighborIsInPhaseandLayer(dir, _phase, L) ) {
              count++;
         }
    }
    return count;
}
void BoundedObjCoating::setLayerNumber(const int _Lnumber){
    Q_ASSERT(_Lnumber==0 || _Lnumber==1);
    for(int label = 0; label < 10; label++){
        outFlags[label].Lnumber= _Lnumber;
    }
}

int BoundedObjCoating::CountFinishedSides(const int _leftDir, const int _rightDir) const
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

int BoundedObjCoating::countRetiredareFinished(const int _Lnumber) const
{
    int countFinished =0;
    for(int dir = 0; dir < 10; dir++) {
        if(neighborIsInPhaseandLayer(dir, Phase::retiredLeader, _Lnumber) && inFlags[dir]->NumFinishedNeighbors == 2) {
              countFinished++; //number of retired leaders p has with layer number =0

        }
    }

    return countFinished;
}

void BoundedObjCoating::setNumFinishedNeighbors(const int _NumFinishedNeighbors)
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].NumFinishedNeighbors = _NumFinishedNeighbors;
    }
}

}
