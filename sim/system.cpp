#include <algorithm>
#include <chrono>
#include <set>
#include <utility>

#include <QDebug>

#include "sim/system.h"

System::System()
    : systemState(SystemState::Valid),
      numMovements(0),
      numRounds(0)
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

System::System(const System& other)
    : rng(other.rng),
      systemState(other.systemState),
      disconnectionNode(other.disconnectionNode),
      numMovements(other.numMovements),
      numRounds(other.numRounds)
{
    for(auto it = other.particles.cbegin(); it != other.particles.cend(); ++it) {
        insert(*it);
    }
}

System& System::operator=(const System& other)
{
    rng = other.rng;
    systemState = other.systemState;
    disconnectionNode = other.disconnectionNode;
    numMovements = other.numMovements;
    numRounds = other.numRounds;
    for(auto it = other.particles.cbegin(); it != other.particles.cend(); ++it) {
        insert(*it);
    }
    return (*this);
}

System::SystemState System::insert(const Particle& p)
{
    Q_ASSERT(particleMap.find(p.head) == particleMap.end());
    Q_ASSERT(p.tailDir == -1 || particleMap.find(p.tail()) == particleMap.end());

    particles.push_back(p);
    particleMap.insert(std::pair<Node, Particle*>(p.head, &particles.back()));
    if(p.tailDir != -1) {
        particleMap.insert(std::pair<Node, Particle*>(p.tail(), &particles.back()));
    }
    return SystemState::Valid;
}

const Particle& System::at(int index) const
{
    return particles.at(index);
}

int System::size() const
{
    return particles.size();
}

System::SystemState System::round()
{
    if(systemState != SystemState::Valid) {
        return systemState;
    }

    if(particles.size() == 0) {
        systemState = SystemState::Terminated;
        return systemState;
    }

    std::deque<Particle*> shuffledParticles;
    for(auto it = particles.begin(); it != particles.end(); ++it) {
        shuffledParticles.push_back(&(*it));
    }
    std::shuffle(shuffledParticles.begin(), shuffledParticles.end(), rng);

    bool hasBlockedParticles = false;
    while(shuffledParticles.size() > 0) {
        Particle* p = shuffledParticles.front();
        updateNumRounds(p);

        auto inFlags = assembleFlags(*p);
        Movement m = p->executeAlgorithm(inFlags);

        if(m.type == MovementType::Empty) {
            p->discard();
            shuffledParticles.pop_front();
            continue;
        } else if(m.type == MovementType::Idle) {
            p->apply();
            return systemState;
        } else if(m.type == MovementType::Expand) {
            if(handleExpansion(*p, m.label)) {
                return systemState;
            }
        } else if(m.type == MovementType::Contract || m.type == MovementType::HandoverContract) {
            if(handleContraction(*p, m.label, m.type == MovementType::HandoverContract)) {
                return systemState;
            }
        }

        // particle is blocked, it can not exceute its action
        hasBlockedParticles = true;
        shuffledParticles.pop_front();
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
    return systemState;
}

System::SystemState System::roundPermutation()
{
    if(systemState != SystemState::Valid) {
        return systemState;
    }

    if(particles.size() == 0) {
        systemState = SystemState::Terminated;
        return systemState;
    }

    std::deque<Particle*> shuffledParticles;
    for(auto it = particles.begin(); it != particles.end(); ++it) {
        shuffledParticles.push_back(&(*it));
    }
    std::shuffle(shuffledParticles.begin(), shuffledParticles.end(), rng);

    bool hasBlockedParticles = false;
    bool somethingActed = false;
    while(shuffledParticles.size() > 0) {
        Particle* p = shuffledParticles.front();
        updateNumRounds(p);

        auto inFlags = assembleFlags(*p);
        Movement m = p->executeAlgorithm(inFlags);

        if(m.type == MovementType::Empty) {
            p->discard();
            shuffledParticles.pop_front();
            continue;
        } else if(m.type == MovementType::Idle) {
            p->apply();
            somethingActed = true;
            shuffledParticles.pop_front();
            continue;
        } else if(m.type == MovementType::Expand) {
            if(handleExpansion(*p, m.label)) {
                somethingActed = true;
            }
            shuffledParticles.pop_front();
            continue;
        } else if(m.type == MovementType::Contract || m.type == MovementType::HandoverContract) {
            if(handleContraction(*p, m.label, m.type == MovementType::HandoverContract)) {
                somethingActed = true;
            }
            shuffledParticles.pop_front();
            continue;
        }

        // particle is blocked, it can not exceute its action
        hasBlockedParticles = true;
        shuffledParticles.pop_front();
    }

    if(somethingActed) {
        systemState = SystemState::Valid;
    } else {
        if(hasBlockedParticles) {
            systemState = SystemState::Deadlocked;
        } else {
            systemState = SystemState::Terminated;
        }
    }
    return systemState;
}

System::SystemState System::roundForParticle(const Node node)
{
    if(systemState != SystemState::Valid) {
        return systemState;
    }

    if(particles.size() == 0) {
        systemState = SystemState::Terminated;
        return systemState;
    }

    auto it = particleMap.find(node);
    if(it == particleMap.end()) {
        return systemState;
    }

    Particle* p = it->second;
    updateNumRounds(p);

    auto inFlags = assembleFlags(*p);
    Movement m = p->executeAlgorithm(inFlags);

    if(m.type == MovementType::Empty) {
        p->discard();
    } else if(m.type == MovementType::Idle) {
        p->apply();
    } else if(m.type == MovementType::Expand) {
        handleExpansion(*p, m.label);
    } else if(m.type == MovementType::Contract || m.type == MovementType::HandoverContract) {
        handleContraction(*p, m.label, m.type == MovementType::HandoverContract);
    }
    return systemState;
}

System::SystemState System::getSystemState() const
{
    return systemState;
}

Node System::getDisconnectionNode() const
{
    return disconnectionNode;
}

int System::getNumMovements() const
{
    return numMovements;
}

int System::getNumRounds() const
{
    return numRounds;
}

std::array<const Flag*, 10> System::assembleFlags(Particle& p)
{
    std::array<const Flag*, 10> flags;    

    int labelLimit = p.tailDir == -1 ? 6 : 10;
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
            flags[label] = neighborIt->second->getFlagForNodeInDir(incidentNode, (p.labelToDir(label) + 3) % 6);
        }
    }

    return flags;
}

bool System::handleExpansion(Particle& p, int label)
{
    if(p.tailDir != -1) {
        p.discard(); // already expanded particle cannot expand
        return false;
    }

    if(label < 0 || label > 5) {
        p.discard(); // invalid expansion index
        return false;
    }

    label = Particle::posMod<6>(p.orientation + label);
    Node newHead = p.head.nodeInDir(label);

    auto otherParticleIt = particleMap.find(newHead);
    if(otherParticleIt == particleMap.end()) {
        // expansion into empty node
        particleMap.insert(std::pair<Node, Particle*>(newHead, &p));
        p.head = newHead;
        p.tailDir = Particle::posMod<6>(label + 3);
        p.apply();
        numMovements++;
        return true;
    } else {
        Particle* otherParticle = otherParticleIt->second;
        if(otherParticle->tailDir == -1 || otherParticle->tail() != newHead) {
            // collision
            p.discard();
            return false;
        } else {
            // attempt push
            auto inFlags = assembleFlags(*otherParticle);
            Movement m = otherParticle->executeAlgorithm(inFlags);
            if(m.type == MovementType::HandoverContract || m.type == MovementType::Contract) {
                // push succeeded
                particleMap.erase(newHead);
                otherParticle->tailDir = -1;
                otherParticle->apply();

                particleMap.insert(std::pair<Node, Particle*>(newHead, &p));
                p.head = newHead;
                p.tailDir = Particle::posMod<6>(label + 3);
                p.apply();

                numMovements += 2;
                return true;
            } else {
                // push failed
                p.discard();
                return false;
            }
        }
    }
}

bool System::handleContraction(Particle& p, int label, bool isHandoverContraction)
{
    if(p.tailDir == -1) {
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
    Particle* handoverParticle = nullptr;
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
            Particle& p2 = *mapIt->second;

            // check whether p2 wants to expand and into which node
            if(p2.tailDir != -1) {
                continue; // already expanded particle cannot expand
            }    
            auto inFlags = assembleFlags(p2);
            Movement m = p2.executeAlgorithm(inFlags);
            if(m.type != MovementType::Expand) {
                p2.discard();
                continue; // we are only interested in expanding particles
            }
            if(m.label < 0 || m.label > 5) {
                p2.discard();
                continue; // invalid expansion index
            }
            int expandDir = Particle::posMod<6>(p2.orientation + m.label);
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
    p.tailDir = -1;
    p.apply();
    numMovements++;
    particleMap.erase(handoverNode);
    if(handover) {
        handoverParticle->head = handoverNode;
        handoverParticle->tailDir = Particle::posMod<6>(handoverExpandDir + 3);
        handoverParticle->apply();
        numMovements++;
        particleMap.insert(std::pair<Node, Particle*>(handoverNode, handoverParticle));
    } else {
        // an isolated contraction is the only action that can disconnect the system
        if(!isConnected()) {
            systemState = SystemState::Disconnected;
            disconnectionNode = handoverNode;
        }
    }
    return true;
}

bool System::isConnected() const
{
    if(particleMap.empty()) {
        return true;
    }

    std::set<Node> occupiedNodes;
    for(auto it = particleMap.cbegin(); it != particleMap.cend(); ++it) {
        occupiedNodes.insert(it->first);
    }

    std::deque<Node> queue;
    queue.push_back(*occupiedNodes.begin());

    while(!queue.empty()) {
        Node n = queue.front();
        queue.pop_front();
        for(int dir = 0; dir < 6; ++dir) {
            Node neighbor = n.nodeInDir(dir);
            auto nondeIt = occupiedNodes.find(neighbor);
            if(nondeIt != occupiedNodes.end()) {
                queue.push_back(neighbor);
                occupiedNodes.erase(nondeIt);
            }
        }
    }

    return occupiedNodes.empty();
}

void System::updateNumRounds(Particle *p)
{
    activatedParticles.insert(p);
    if(activatedParticles.size() == particles.size()) {
        numRounds++;
        activatedParticles.clear();
    }
}
