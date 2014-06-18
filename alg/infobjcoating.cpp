#include <algorithm>

#include "alg/infobjcoating.h"
#include "sim/system.h"

InfObjCoating::InfObjCoating(const Phase _phase)
{
    initFlags<InfObjCoatingFlag>();
    outFlags = castFlags<InfObjCoatingFlag>(Algorithm::outFlags);
    setPhase(_phase);
}

InfObjCoating::InfObjCoating(const InfObjCoating& other)
    : Algorithm(other),
      phase(other.phase),
      followDir(other.followDir)
{
    copyFlags<InfObjCoatingFlag>(other);
}

InfObjCoating::~InfObjCoating()
{
    deleteFlags();
}

System* InfObjCoating::instance(const int size)
{
    System* system = new System();
    for(int x = 0; x < size; x++) {
        Phase phase = x == size - 1 ? Phase::Lead : Phase::Inactive;
        int orientation = size - 1 ? 0 : randDir();
        Node position = Node(x - size / 2, 0);
        int tailDir = -1;
        system->insert(Particle(new InfObjCoating(phase), orientation, position, tailDir));
    }
    return system;
}

Algorithm* InfObjCoating::clone()
{
    return new InfObjCoating(*this);
}

bool InfObjCoating::isDeterministic() const
{
    return true;
}

Movement InfObjCoating::execute(std::array<const Flag*, 10>& flags)
{
    inFlags = castFlags<InfObjCoatingFlag>(flags);
    outFlags = castFlags<InfObjCoatingFlag>(Algorithm::outFlags);

    if(isExpanded()) {
        if(phase == Phase::Follow) {
            setFollowIndicatorLabel(followDir);
        }

        if(hasNeighborInPhase(Phase::Inactive) || receivesFollowIndicator()) {
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        } else {
            return Movement(MovementType::Contract, tailContractionLabel());
        }
    } else {
        if(phase == Phase::Inactive) {
            auto label = std::max(neighborInPhase(Phase::Follow), neighborInPhase(Phase::Lead));
            if(label != -1) {
                setPhase(Phase::Follow);
                followDir = labelToDir(label);
                setFollowIndicatorLabel(followDir);
                headMarkDir = followDir;
                return Movement(MovementType::Idle);
            }
        } else if(phase == Phase::Follow) {
            Q_ASSERT(inFlags[followDir] != nullptr);
            if(inFlags[followDir]->isExpanded()) {
                int oldFollowDir = followDir;
                setContractDir(oldFollowDir);
                followDir = updatedFollowDir();
                setFollowIndicatorLabel(dirToHeadLabelAfterExpansion(followDir, oldFollowDir));
                headMarkDir = followDir;
                return Movement(MovementType::Expand, oldFollowDir);
            }
        } else if(phase == Phase::Lead) {
            setContractDir(dirToHeadLabelAfterExpansion(1, 1));
            return Movement(MovementType::Expand, 1);
        }

        return Movement(MovementType::Empty);
    }
}

void InfObjCoating::setPhase(const Phase _phase)
{
    phase = _phase;

    if(phase == Phase::Lead) {
        headMarkColor = 0xff0000;
        tailMarkColor = 0xff0000;
    } else if(phase == Phase::Follow) {
        headMarkColor = 0x0000ff;
        tailMarkColor = 0x0000ff;
    } else { // phase == Phase::Inactive
        headMarkColor = -1;
        tailMarkColor = -1;
    }

    for(int label = 0; label < 10; label++) {
        outFlags[label]->phase = phase;
    }
}

int InfObjCoating::neighborInPhase(const Phase _phase) const
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr) {
            if(inFlags[label]->phase == _phase) {
                return label;
            }
        }
    }
    return -1;
}

bool InfObjCoating::hasNeighborInPhase(const Phase _phase) const
{
    return (neighborInPhase(_phase) != -1);
}

void InfObjCoating::setContractDir(const int contractDir)
{
    for(int label = 0; label < 10; label++) {
        outFlags[label]->contractDir = contractDir;
    }
}

int InfObjCoating::updatedFollowDir() const
{
    int contractDir = inFlags[followDir]->contractDir;
    int offset = (inFlags[followDir]->dir - followDir + 9) % 6;
    int tempFollowDir = (contractDir + offset) % 6;
    Q_ASSERT(0 <= tempFollowDir && tempFollowDir <= 5);
    return tempFollowDir;
}

void InfObjCoating::setFollowIndicatorLabel(const int label)
{
    for(int i = 0; i < 10; i++) {
        outFlags[i]->followIndicator = (i == label);
    }
}

bool InfObjCoating::receivesFollowIndicator() const
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr) {
            if(inFlags[label]->followIndicator) {
                return true;
            }
        }
    }
    return false;
}

InfObjCoatingFlag::InfObjCoatingFlag()
    : followIndicator(false)
{
}

InfObjCoatingFlag::InfObjCoatingFlag(const InfObjCoatingFlag& other)
    : Flag(other),
      phase(other.phase),
      contractDir(other.contractDir),
      followIndicator(other.followIndicator)
{
}
