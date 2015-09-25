#include <chrono>

#include "legacysystem.h"

LegacySystem::LegacySystem() :
    systemState(SystemState::Valid),
    numNonStaticParticles(0),
    _numMovements(0),
    _numRounds(0)
{
    uint32_t seed;
    std::random_device device;
    if(device.entropy() == 0) {
        auto duration = std::chrono::high_resolution_clock::now() - std::chrono::high_resolution_clock::time_point::min();
        seed = duration.count();
    } else {
        std::uniform_int_distribution<uint32_t> dist(std::numeric_limits<uint32_t>::min(),
                                                     std::numeric_limits<uint32_t>::max());
        seed = dist(device);
    }
    rng.seed(seed);
}

void LegacySystem::activate(){
    if(systemState != SystemState::Valid) {
        return;
    }

    if(numNonStaticParticles == 0) {
        systemState = SystemState::Terminated;
        return;
    }

    bool attemptedActivatingEveryParticles = false;
    bool hasBlockedParticles = false;

    while(!attemptedActivatingEveryParticles) {
        if(shuffledParticles.size() == 0) {
            attemptedActivatingEveryParticles = true;

            for(auto it = particles.begin(); it != particles.end(); ++it) {
                shuffledParticles.push_back(&(*it));
            }
            std::shuffle(shuffledParticles.begin(), shuffledParticles.end(), rng);
        }

        while(shuffledParticles.size() > 0) {
            LegacyParticle* p = shuffledParticles.front();
            shuffledParticles.pop_front();

            if(p->isStatic()) {
                continue;
            }

            updateNumRounds(p);

            auto inFlags = assembleFlags(*p);
            Movement m = p->executeAlgorithm(inFlags);

            if(m.type == MovementType::Empty) {
                p->discard();
                continue;
            } else if(m.type == MovementType::Idle) {
                p->apply();
                return;
            } else if(m.type == MovementType::Expand) {
                if(handleExpansion(*p, m.label)) {
                    return;
                }
            } else if(m.type == MovementType::Contract || m.type == MovementType::HandoverContract) {
                if(handleContraction(*p, m.label, m.type == MovementType::HandoverContract)) {
                    return;
                }
            }

            // particle is blocked, it cannot execute its action
            hasBlockedParticles = true;
        }
    }

    if(hasBlockedParticles) {
        if(particles[0].algorithmIsDeterministic()) {
            systemState = SystemState::Deadlocked;
        } else {
            systemState = SystemState::Valid;
        }
    } else {
        systemState = SystemState::Terminated;
    }

    return;
}

unsigned int LegacySystem::size() const
{
    return particles.size();
}

const LegacyParticle& LegacySystem::at(const int i) const
{
    return particles.at(i);
}

int LegacySystem::numMovements() const
{
    return _numMovements;
}

int LegacySystem::numRounds() const
{
    return _numRounds;
}

bool LegacySystem::hasTerminated() const
{
    return systemState != SystemState::Valid;
}

void LegacySystem::insertParticle(const LegacyParticle& p)
{
    Q_ASSERT(particleMap.find(p.head) == particleMap.end());
    Q_ASSERT(p.globalTailDir == -1 || particleMap.find(p.tail()) == particleMap.end());

    particles.push_back(p);
    particleMap.insert(std::pair<Node, LegacyParticle*>(p.head, &particles.back()));
    if(p.globalTailDir != -1) {
        particleMap.insert(std::pair<Node, LegacyParticle*>(p.tail(), &particles.back()));
    }

    if(!p.isStatic()) {
        numNonStaticParticles++;
    }
}

std::array<const Flag*, 10> LegacySystem::assembleFlags(LegacyParticle& p){
    std::array<const Flag*, 10> flags;

    int labelLimit = p.globalTailDir == -1 ? 6 : 10;
    for(int label = 0; label < 10; label++) {
        if(label >= labelLimit) {
            flags[label] = nullptr;
            continue;
        }

        auto neighborIt = particleMap.find(p.neighboringNodeReachedViaLabel(label));
        if(neighborIt == particleMap.end()) {
            flags[label] = nullptr;
        } else {
            auto incidentNode = p.occupiedNodeIncidentToLabel(label);
            flags[label] = neighborIt->second->getFlagForNodeInDir(incidentNode, (p.labelToGlobalDir(label) + 3) % 6);
        }
    }

    return flags;
}

bool LegacySystem::handleExpansion(LegacyParticle& p, int label){
    if(p.globalTailDir != -1) {
        p.discard(); // already expanded particle cannot expand
        return false;
    }

    if(label < 0 || label > 5) {
        p.discard(); // invalid expansion index
        return false;
    }

    int expansionDir = posMod<6>(p.orientation + label);
    Node newHead = p.head.nodeInDir(expansionDir);

    auto otherParticleIt = particleMap.find(newHead);
    if(otherParticleIt == particleMap.end() || otherParticleIt->second->isStatic()) {
        // expansion into empty node
        particleMap.insert(std::pair<Node, LegacyParticle*>(newHead, &p));
        p.head = newHead;
        p.globalTailDir = posMod<6>(expansionDir + 3);
        p.apply();
        _numMovements++;
        return true;
    } else {
        LegacyParticle* otherParticle = otherParticleIt->second;
        if(otherParticle->globalTailDir == -1) {
            // collision
            p.discard();
            return false;
        } else {
            // attempt push
            auto inFlags = assembleFlags(*otherParticle);
            Movement m = otherParticle->executeAlgorithm(inFlags);

            // An attempt towards a pull or push also counts as an activation.
            updateNumRounds(otherParticle);

            bool pushSucceeded = false;
            if(m.type == MovementType::HandoverContract || m.type == MovementType::Contract) {
                // determine whether the contraction direction is valid
                // and, if so, whether it is a head or a tail contraction
                if(m.label == otherParticle->headContractionLabel()) {
                    // valid headContraction
                    if(newHead == otherParticle->head) {
                        pushSucceeded = true;
                    }
                } else if(m.label == otherParticle->tailContractionLabel()) {
                    // valid tailContraction
                    if(newHead == otherParticle->tail()) {
                        pushSucceeded = true;
                    }
                }
            }

            if(pushSucceeded) {
                // push succeeded
                particleMap.erase(newHead);
                otherParticle->globalTailDir = -1;
                otherParticle->apply();

                particleMap.insert(std::pair<Node, LegacyParticle*>(newHead, &p));
                p.head = newHead;
                p.globalTailDir = posMod<6>(expansionDir + 3);
                p.apply();

                _numMovements += 2;
                return true;
            } else {
                // push failed
                otherParticle->discard();
                p.discard();
                return false;
            }
        }
    }
}

bool LegacySystem::handleContraction(LegacyParticle& p, int label, bool isHandoverContraction){
    if(p.globalTailDir == -1) {
        p.discard(); // already contracted particle cannot contract
        return false;
    }

    // determine whether the contraction direction is valid
    // and, if so, whether it is a head or a tail contraction
    bool isHeadContract = false;
    if(label == p.headContractionLabel()) {
        isHeadContract = true;
    } else if(label == p.tailContractionLabel()) {
        isHeadContract = false;
    } else {
        p.discard();
        return false;
    }

    bool handover = false;
    LegacyParticle* handoverParticle = nullptr;
    int handoverExpandDir = -1;

    if(isHandoverContraction) {
        // look for a particle that wants to join in the action to do a handover
        std::deque<int> neighborDirs = {0, 1, 2, 3, 4, 5};;
        while(neighborDirs.size() > 0) {
            // select a remaining direction from neighborDirs
            auto dist = std::uniform_int_distribution<int>(0, neighborDirs.size() - 1);
            auto index = dist(rng);
            auto neighborDirIt = neighborDirs.begin() + index;
            auto neighborDir = *neighborDirIt;
            neighborDirs.erase(neighborDirIt);

            // determine the node the direction points to
            // and ensure that this node is not occupied by p
            Node neighbor;
            if(isHeadContract) {
                neighbor = p.head.nodeInDir(neighborDir);
                if(neighbor == p.tail()) {
                    continue;
                }
            } else {
                neighbor = p.tail().nodeInDir(neighborDir);
                if(neighbor == p.head) {
                    continue;
                }
            }

            // determine the (potential) particle p2 that occupies the node
            // and ensure that there indeed is such a particle
            auto mapIt = particleMap.find(neighbor);
            if(mapIt == particleMap.end()) {
                continue;
            }
            LegacyParticle& p2 = *mapIt->second;

            if(p2.isStatic()) {
                continue;
            }

            // check whether p2 wants to expand and into which node
            if(p2.globalTailDir != -1) {
                continue; // already expanded particle cannot expand
            }
            auto inFlags = assembleFlags(p2);
            Movement m = p2.executeAlgorithm(inFlags);

            // An attempt towards a pull or push also counts as an activation.
            // NOTE: This is a little bit unfair. Should failed attempts really count as an activation?
            updateNumRounds(&p2);

            if(m.type != MovementType::Expand) {
                p2.discard();
                continue; // we are only interested in expanding particles
            }
            if(m.label < 0 || m.label > 5) {
                p2.discard();
                continue; // invalid expansion index
            }
            int expandDir = posMod<6>(p2.orientation + m.label);
            Node newHead = p2.head.nodeInDir(expandDir);

            // ensure that the node p2 wants to expand into is the node p wants to contract out of
            if( ( isHeadContract && newHead != p.head) ||
                (!isHeadContract && newHead != p.tail())) {
                p2.discard();
                continue;
            }

            // successful handover!
            // the particle will be modified accordingly at the bottom of this method
            handover = true;
            handoverParticle = &p2;
            handoverExpandDir = expandDir;
            break;
        }
    }

    // a handover contraction can only executed as part of a handover
    if(isHandoverContraction && !handover) {
        p.discard();
        return false;
    }

    // apply changes to particles and particleMap
    Node handoverNode;
    if(isHeadContract) {
        handoverNode = p.head;
        p.head = p.tail();
    } else {
        handoverNode = p.tail();
    }
    p.globalTailDir = -1;
    p.apply();
    _numMovements++;
    particleMap.erase(handoverNode);
    if(handover) {
        handoverParticle->head = handoverNode;
        handoverParticle->globalTailDir = posMod<6>(handoverExpandDir + 3);
        handoverParticle->apply();
        _numMovements++;
        particleMap.insert(std::pair<Node, LegacyParticle*>(handoverNode, handoverParticle));
    } else {
        // an isolated contraction is the only action that can disconnect the system
        std::set<Node> occupiedNodes;
        for(auto& p : particles) {
            occupiedNodes.insert(p.head);
            if(p.globalTailDir != -1) {
                occupiedNodes.insert(p.tail());
            }
        }
        if(!isConnected(occupiedNodes)) {
            systemState = SystemState::Disconnected;
        }
    }
    return true;
}

void LegacySystem::updateNumRounds(LegacyParticle *p)
{
    activatedParticles.insert(p);
    if(activatedParticles.size() == numNonStaticParticles) {
        _numRounds++;
        activatedParticles.clear();
    }
}
