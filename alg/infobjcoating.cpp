#include <algorithm>
#include <cmath>
#include <deque>
#include <set>

#include "alg/infobjcoating.h"
#include "sim/particle.h"
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

System* InfObjCoating::instance(const int numParticles, const float holeProb)
{   
    System* system = new System();

    std::deque<Node> orderedSurface;
    std::set<Node> occupied;

    Node pos;
    int lastOffset = 0;
    while(system->size() < 2 * numParticles) {
        system->insert(Particle(new InfObjCoating(Phase::Static), randDir(), pos));
        occupied.insert(pos);
        orderedSurface.push_back(pos);
        int offset;
        if(lastOffset == 4) {
            offset = randInt(3, 5);
        } else {
            offset = randInt(2, 5);
        }
        lastOffset = offset;
        pos = pos.nodeInDir(offset);
    }

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

    int numNonStaticParticles = 0;
    while(numNonStaticParticles < numParticles) {
        if(candidates.empty()) {
            return system;
        }

        std::set<Node> nextCandidates;
        for(auto it = candidates.begin(); it != candidates.end(); ++it) {
            if(randBool(1.0f - holeProb)) {
                system->insert(Particle(new InfObjCoating(Phase::Inactive), randDir(), *it));
                numNonStaticParticles++;

                for(int dir = 1; dir <= 2; dir++) {
                    const Node node = it->nodeInDir(dir);
                    if(occupied.find(node) == occupied.end()) {
                        nextCandidates.insert(node);
                        occupied.insert(node);
                    }
                }
            }
        }
        nextCandidates.swap(candidates);
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
            auto label = neighborInPhase(Phase::Static);
            if(label != -1) {
                setPhase(Phase::Lead);
                return Movement(MovementType::Idle);
            }

            label = std::max(neighborInPhase(Phase::Follow), neighborInPhase(Phase::Lead));
            if(label != -1) {
                setPhase(Phase::Follow);
                followDir = labelToDir(label);
                setFollowIndicatorLabel(followDir);
                headMarkDir = followDir;
                return Movement(MovementType::Idle);
            }
        } else if(phase == Phase::Follow) {
            Q_ASSERT(inFlags[followDir] != nullptr);
            auto label = neighborInPhase(Phase::Static);
            if(label != -1) {
                setPhase(Phase::Lead);
                return Movement(MovementType::Idle);
            }

            if(inFlags[followDir]->isExpanded()) {
                int oldFollowDir = followDir;
                setContractDir(oldFollowDir);
                followDir = updatedFollowDir();
                setFollowIndicatorLabel(dirToHeadLabelAfterExpansion(followDir, oldFollowDir));
                headMarkDir = followDir;
                return Movement(MovementType::Expand, oldFollowDir);
            }
        } else if(phase == Phase::Lead) {
//            setContractDir(dirToHeadLabelAfterExpansion(1, 1));
//            return Movement(MovementType::Expand, 1);
            return Movement(MovementType::Empty);
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
    } else if(phase == Phase::Inactive) {
        headMarkColor = -1;
        tailMarkColor = -1;
    } else { // phase == Phase::Static
        headMarkColor = 0x000000;
        tailMarkColor = 0x000000;
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
