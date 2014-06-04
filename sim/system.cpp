#include <chrono>

#include <QDebug>

#include "alg/algorithm.h"
#include "sim/system.h"

System::System()
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
    : particles(other.particles)
{
}

bool System::insert(const Particle& p)
{
    if(particleMap.find(p.headPos) != particleMap.end()) {
        return false;
    }

    if(p.tailDir != -1 && particleMap.find(p.tailPos()) != particleMap.end()) {
        return false;
    }

    particles.push_back(p);
    particleMap.insert(std::pair<Vec, Particle&>(p.headPos, particles.back()));
    if(p.tailDir != -1) {
        particleMap.insert(std::pair<Vec, Particle&>(p.tailPos(), particles.back()));
    }
    return true;
}

const Particle& System::at(int index) const
{
    return particles.at(index);
}

int System::size() const
{
    return particles.size();
}

void System::round()
{
    if(particles.size() == 0) {
        return;
    }

    auto dist = std::uniform_int_distribution<int>(0, particles.size() - 1);
    auto index = dist(rng);

    Particle& p = particles[index];
    Particle backup = p;
    Movement m = p.executeAlgorithm();

    if(m.type == MovementType::Expand) {
        bool success = handleExpansion(p, m.dir);
        if(!success) {
            p = backup;
        }
    } else if(m.type == MovementType::Contract || m.type == MovementType::HandoverContract) {
        bool success = handleContraction(p, m.dir, m.type == MovementType::HandoverContract);
        if(!success) {
            p = backup;
        }
    }
}

bool System::handleExpansion(Particle& p, int dir)
{
    if(p.tailDir != -1) {
        return false; // already expanded particle cannot expand
    }

    if(dir < 0 || dir > 5) {
        return false; // invalid expansion index
    }

    dir = posMod(p.orientation + dir, 6);
    Vec newHeadPos = p.headPos.vecInDir(dir);

    if(particleMap.find(newHeadPos) != particleMap.end()) {
        return false; // collision
    }

    particleMap.insert(std::pair<Vec, Particle&>(newHeadPos, p));
    p.headPos = newHeadPos;
    p.tailDir = posMod(dir + 3, 6);
    return true;
}

bool System::handleContraction(Particle& p, int dir, bool isHandoverContraction)
{
    if(p.tailDir == -1) {
        return false ; // already contracted particle cannot contract
    }

    // determine whether the contraction direction is valid
    // and, if so, whether it is a head or a tail contraction
    bool isHeadContract;
    if(p.tailDir == p.orientation) {
        if(dir == 0) {          // head contraction
            isHeadContract = true;
        } else if(dir == 5) {   // tail contraction
            isHeadContract = false;
        } else {                // invalid contraction
            return false;
        }
    } else if(p.tailDir == (p.orientation + 1) % 6) {
        if(dir == 1) {          // head contraction
            isHeadContract = true;
        } else if(dir == 6) {   // tail contraction
            isHeadContract = false;
        } else {                // invalid contraction
            return false;
        }
    } else if(p.tailDir == (p.orientation + 2) % 6) {
        if(dir == 4) {          // head contraction
            isHeadContract = true;
        } else if(dir == 9) {   // tail contraction
            isHeadContract = false;
        } else {                // invalid contraction
            return false;
        }
    } else if(p.tailDir == (p.orientation + 3) % 6) {
        if(dir == 5) {          // head contraction
            isHeadContract = true;
        } else if(dir == 0) {   // tail contraction
            isHeadContract = false;
        } else {                // invalid contraction
            return false;
        }
    } else if(p.tailDir == (p.orientation + 4) % 6) {
        if(dir == 6) {          // head contraction
            isHeadContract = true;
        } else if(dir == 1) {   // tail contraction
            isHeadContract = false;
        } else {                // invalid contraction
            return false;
        }
    } else if(p.tailDir == (p.orientation + 5) % 6) {
        if(dir == 9) {          // head contraction
            isHeadContract = true;
        } else if(dir == 4) {   // tail contraction
            isHeadContract = false;
        } else {                // invalid contraction
            return false;
        }
    }

    // look for a particle that wants to join in the action to do a handover
    // if such a particle is found, do a handover
    bool handover = false;
    Particle* handoverParticle = nullptr;
    int handoverExpandDir = -1;

    auto dist = std::uniform_int_distribution<int>(0, 1);
    bool attemptHandover = dist(rng);

    if(isHandoverContraction || attemptHandover) {
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
            Vec neighbor;
            if(isHeadContract) {
                neighbor = p.headPos.vecInDir(neighborDir);
                if(neighbor == p.tailPos()) {
                    continue;
                }
            } else {
                neighbor = p.tailPos().vecInDir(neighborDir);
                if(neighbor == p.headPos) {
                    continue;
                }
            }

            // determine the (potential) particle p2 that occupies the node
            // and ensure that there indeed is such a particle
            auto mapIt = particleMap.find(neighbor);
            if(mapIt == particleMap.end()) {
                continue;
            }
            Particle& p2 = mapIt->second;

            // check whether p2 wants to expand and into which node
            if(p2.tailDir != -1) {
                continue;
            }
            Particle backup = p2;
            Movement m = p2.executeAlgorithm();
            if(m.type != MovementType::Expand) {
                p2 = backup;
                continue;
            }
            if(m.dir < 0 || m.dir > 5) {
                p2 = backup;
                continue;
            }
            int expandDir = posMod(p2.orientation + m.dir, 6);
            Vec newHeadPos = p2.headPos.vecInDir(expandDir);

            // ensure that the node p2 wants to expand into is the node p wants to contract out of
            if( (isHeadContract && newHeadPos != p.headPos) ||
                (!isHeadContract && newHeadPos != p.tailPos())) {
                p2 = backup;
                continue;
            }

            // successful handover! the particle will be changed accordingly below
            handover = true;
            handoverParticle = &p2;
            handoverExpandDir = expandDir;
            break;
        }
    }

    // a handover contraction can only executed as part of a handover
    if(isHandoverContraction && !handover) {
        return false;
    }

    // apply changes to particles and particleMap
    Vec handoverNode;
    if(isHeadContract) {
        handoverNode = p.headPos;
        p.headPos = p.tailPos();
    } else {
        handoverNode = p.tailPos();
    }
    p.tailDir = -1;
    particleMap.erase(handoverNode);
    if(handover) {
        handoverParticle->headPos = handoverNode;
        handoverParticle->tailDir = posMod(handoverExpandDir + 3, 6);
        particleMap.insert(std::pair<Vec, Particle&>(handoverNode, *handoverParticle));
    }

    return true;
}
