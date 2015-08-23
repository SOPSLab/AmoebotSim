#include <algorithm>
#include <cmath>
#include <deque>
#include <set>

#include "alg/infobjcoating.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace InfObjCoating
{
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

InfObjCoating::InfObjCoating(const Phase _phase)
{
    setPhase(_phase);
}

InfObjCoating::InfObjCoating(const InfObjCoating& other)
    : AlgorithmWithFlags(other),
      phase(other.phase),
      followDir(other.followDir)
{
}

InfObjCoating::~InfObjCoating()
{
}

std::shared_ptr<System> InfObjCoating::instance(const int numParticles, const float holeProb)
{
    std::shared_ptr<System> system = std::make_shared<System>();

    std::deque<Node> orderedSurface;
    std::set<Node> occupied;

    Node pos;
    int lastOffset = 0;
    while(system->size() < 2 * numParticles) {
        system->insert(Particle(std::make_shared<InfObjCoating>(Phase::Static), randDir(), pos));
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
        for(auto it = candidates.begin(); it != candidates.end() && numNonStaticParticles < numParticles; ++it) {
            if(randBool(1.0f - holeProb)) {
                system->insert(Particle(std::make_shared<InfObjCoating>(Phase::Inactive), randDir(), *it));
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

Movement InfObjCoating::execute()
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
    } else {
        if(phase == Phase::Inactive) {
            if(hasNeighborInPhase(Phase::Static)) {
                setPhase(Phase::Set);
                return Movement(MovementType::Idle);
            }
            else if (hasNeighborInPhase(Phase::Set)){
                setPhase(Phase::Lead);
                return Movement(MovementType::Idle);
            }
            auto label = std::max(firstNeighborInPhase(Phase::Follow), firstNeighborInPhase(Phase::Lead));
            if(label != -1) {
                setPhase(Phase::Follow);
                followDir = labelToDir(label);
                setFollowIndicatorLabel(followDir);
                headMarkDir = followDir;
                return Movement(MovementType::Idle);
            }
        } else if(phase == Phase::Follow) {
            Q_ASSERT(inFlags[followDir] != nullptr);
            if(hasNeighborInPhase(Phase::Set)) {
                setPhase(Phase::Lead);
                unsetFollowIndicator();
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
            if (hasNeighborInPhase(Phase::Static)) {
                setPhase(Phase::Set);
                headMarkDir = -1;
                return Movement(MovementType::Idle);
            }
            else{
                int moveDir = getMoveDir();
                setContractDir(moveDir);
                headMarkDir = moveDir;
                return Movement(MovementType::Expand, moveDir);
            }
        } 
        return Movement(MovementType::Empty);
    }
}

std::shared_ptr<Algorithm> InfObjCoating::blank()
{
    return std::make_shared<InfObjCoating>(Phase::Inactive);
}

std::shared_ptr<Algorithm> InfObjCoating::clone()
{
    return std::make_shared<InfObjCoating>(*this);
}

bool InfObjCoating::isDeterministic() const
{
    return true;
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
    } else if(phase == Phase::Set){
        headMarkColor = 0x00ff00;
        tailMarkColor = 0x00ff00;
    } else if(phase == Phase::Inactive) {
        headMarkColor = -1;
        tailMarkColor = -1;
    } else { // phase == Phase::Static
        headMarkColor = 0x000000;
        tailMarkColor = 0x000000;
    }

    for(int label = 0; label < 10; label++) {
        outFlags[label].phase = phase;
    }
}

bool InfObjCoating::neighborIsInPhase(const int label, const Phase _phase) const
{
    Q_ASSERT(0 <= label && label <= 9);
    return (inFlags[label] != nullptr && inFlags[label]->phase == _phase);
}

int InfObjCoating::firstNeighborInPhase(const Phase _phase) const
{
    for(int label = 0; label < 10; label++) {
        if(neighborIsInPhase(label, _phase)) {
            return label;
        }
    }
    return -1;
}

bool InfObjCoating::hasNeighborInPhase(const Phase _phase) const
{
    return (firstNeighborInPhase(_phase) != -1);
}

void InfObjCoating::setContractDir(const int contractDir)
{
    for(int label = 0; label < 10; label++) {
        outFlags[label].contractDir = contractDir;
    }
}

int InfObjCoating::updatedFollowDir() const
{
    int contractDir = inFlags[followDir]->contractDir;
    int offset = (followDir - inFlags[followDir]->dir + 9) % 6;
    int tempFollowDir = (contractDir + offset) % 6;
    Q_ASSERT(0 <= tempFollowDir && tempFollowDir <= 5);
    return tempFollowDir;
}

void InfObjCoating::unsetFollowIndicator()
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = false;
    }
}

void InfObjCoating::setFollowIndicatorLabel(const int label)
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = (i == label);
    }
}

bool InfObjCoating::tailReceivesFollowIndicator() const
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

int InfObjCoating::getMoveDir() const
{
    Q_ASSERT(isContracted());
    int label = firstNeighborInPhase(Phase::Set);
    label = (label+5)%6;
    while (neighborIsInPhase(label, Phase::Set)){
        label = (label+5)%6;
    }
    return labelToDir(label);
}
}
