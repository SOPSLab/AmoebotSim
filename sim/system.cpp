#include <algorithm>
#include <chrono>
#include <set>
#include <utility>

#include <QDebug>

#include "sim/system.h"

System::System()
    : systemState(SystemState::Valid)
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
      systemState(other.systemState)
{
    for(auto it = other.particles.cbegin(); it != other.particles.cend(); ++it) {
        insert(*it);
    }
}

System& System::operator=(const System& other)
{
    rng = other.rng;
    systemState = other.systemState;
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
        auto inFlags = assembleFlags(*p);
        Movement m = p->executeAlgorithm(inFlags);

        if(m.type == MovementType::Empty) {
            // particle becomes inactive voluntarily
            // it will be activated once its neighborhood changes
            p->discard();
            shuffledParticles.pop_front();
            continue;
        } else if(m.type == MovementType::Idle) {
            p->apply();
            return systemState;
        } else if(m.type == MovementType::Expand) {
            if(handleExpansion(*p, m.dir)) {
                return systemState;
            }
        } else if(m.type == MovementType::Contract || m.type == MovementType::HandoverContract) {
            if(handleContraction(*p, m.dir, m.type == MovementType::HandoverContract)) {
                return systemState;
            }
        }

        // particle is blocked, it can not exceute its action
        hasBlockedParticles = true;
        shuffledParticles.pop_front();
    }

    if(hasBlockedParticles) {
        if(particles[0].algorithmIsDeterministic()) {
            systemState = SystemState::Deadlock;
        } else {
            systemState = SystemState::Valid;
        }
    } else {
        systemState = SystemState::Terminated;
    }
    return systemState;
}

System::SystemState System::getSystemState() const
{
    return systemState;
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
            flags[label] = neighborIt->second->getFlagForNode(incidentNode);
        }
    }

    return flags;
}

bool System::handleExpansion(Particle& p, int dir)
{
    if(p.tailDir != -1) {
        p.discard(); // already expanded particle cannot expand
        return false;
    }

    if(dir < 0 || dir > 5) {
        p.discard(); // invalid expansion index
        return false;
    }

    dir = Particle::posMod<6>(p.orientation + dir);
    Node newHead = p.head.nodeInDir(dir);

    if(particleMap.find(newHead) != particleMap.end()) {
        p.discard(); // collision
        return false;
    }

    particleMap.insert(std::pair<Node, Particle*>(newHead, &p));
    p.head = newHead;
    p.tailDir = Particle::posMod<6>(dir + 3);
    p.apply();
    return true;
}

bool System::handleContraction(Particle& p, int dir, bool isHandoverContraction)
{
    if(p.tailDir == -1) {
        p.discard(); // already contracted particle cannot contract
        return false;
    }

    // determine whether the contraction direction is valid
    // and, if so, whether it is a head or a tail contraction
    bool isHeadContract = false;
    if(p.tailDir == p.orientation) {
        if(dir == 0) {          // head contraction
            isHeadContract = true;
        } else if(dir == 5) {   // tail contraction
            isHeadContract = false;
        } else {                // invalid contraction
            p.discard();
            return false;
        }
    } else if(p.tailDir == (p.orientation + 1) % 6) {
        if(dir == 1) {          // head contraction
            isHeadContract = true;
        } else if(dir == 6) {   // tail contraction
            isHeadContract = false;
        } else {                // invalid contraction
            p.discard();
            return false;
        }
    } else if(p.tailDir == (p.orientation + 2) % 6) {
        if(dir == 4) {          // head contraction
            isHeadContract = true;
        } else if(dir == 9) {   // tail contraction
            isHeadContract = false;
        } else {                // invalid contraction
            p.discard();
            return false;
        }
    } else if(p.tailDir == (p.orientation + 3) % 6) {
        if(dir == 5) {          // head contraction
            isHeadContract = true;
        } else if(dir == 0) {   // tail contraction
            isHeadContract = false;
        } else {                // invalid contraction
            p.discard();
            return false;
        }
    } else if(p.tailDir == (p.orientation + 4) % 6) {
        if(dir == 6) {          // head contraction
            isHeadContract = true;
        } else if(dir == 1) {   // tail contraction
            isHeadContract = false;
        } else {                // invalid contraction
            p.discard();
            return false;
        }
    } else if(p.tailDir == (p.orientation + 5) % 6) {
        if(dir == 9) {          // head contraction
            isHeadContract = true;
        } else if(dir == 4) {   // tail contraction
            isHeadContract = false;
        } else {                // invalid contraction
            p.discard();
            return false;
        }
    }

    bool handover = false;
    Particle* handoverParticle = nullptr;
    int handoverExpandDir = -1;

    // decide whether a handover should be attempted
    auto dist = std::uniform_int_distribution<int>(0, 1);
    bool attemptHandover = isHandoverContraction || dist(rng);
    if(attemptHandover) {
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
            if(m.dir < 0 || m.dir > 5) {
                p2.discard();
                continue; // invalid expansion index
            }
            int expandDir = Particle::posMod<6>(p2.orientation + m.dir);
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
    particleMap.erase(handoverNode);
    if(handover) {
        handoverParticle->head = handoverNode;
        handoverParticle->tailDir = Particle::posMod<6>(handoverExpandDir + 3);
        handoverParticle->apply();
        particleMap.insert(std::pair<Node, Particle*>(handoverNode, handoverParticle));
    } else {
        if(!isConnected()) {
            systemState = SystemState::Disconnected;
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
