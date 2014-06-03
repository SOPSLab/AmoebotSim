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
    Particle before = p;
    Movement m = p.executeAlgorithm();

    if(m.type == MovementType::Expand) {
        if(p.tailDir != -1) {
            qDebug() << "already expanded particle cannot expand";
            p = before;
        } else if(0 <= m.dir && m.dir <= 5) {
            int dir = posMod(p.orientation + m.dir, 6);
            Vec newHeadPos = p.headPos.vecInDir(dir);
            if(particleMap.find(newHeadPos) == particleMap.end()) {
                particleMap.insert(std::pair<Vec, Particle&>(newHeadPos, p));
                p.headPos = newHeadPos;
                p.tailDir = posMod(dir + 3, 6);
            } else {
                qDebug() << "collision";
                p = before;
            }
        } else {
            qDebug() << "invalid expansion index";
            p = before;
        }
    } else if(m.type == MovementType::Contract || m.type == MovementType::HandoverContract) {
        if(p.tailDir == -1) {
            qDebug() << "already contracted particle cannot contract";
            p = before;
            return;
        }

        bool isHeadContract;
        if(p.tailDir == p.orientation) {
            if(m.dir == 0) {        // head contraction
                particleMap.erase(p.headPos);
                p.headPos = p.tailPos();
                p.tailDir = -1;
                isHeadContract = true;
            } else if(m.dir == 5) { // tail contraction
                particleMap.erase(p.tailPos());
                p.tailDir = -1;
                isHeadContract = false;
            } else {                // invalid contraction
                qDebug() << "invalid contraction index";
                p = before;
                return;
            }
        } else if(p.tailDir == (p.orientation + 1) % 6) {
            if(m.dir == 1) {        // head contraction
                particleMap.erase(p.headPos);
                p.headPos = p.tailPos();
                p.tailDir = -1;
                isHeadContract = true;
            } else if(m.dir == 6) { // tail contraction
                particleMap.erase(p.tailPos());
                p.tailDir = -1;
                isHeadContract = false;
            } else {                // invalid contraction
                qDebug() << "invalid contraction index";
                p = before;
                return;
            }
        } else if(p.tailDir == (p.orientation + 2) % 6) {
            if(m.dir == 4) {        // head contraction
                particleMap.erase(p.headPos);
                p.headPos = p.tailPos();
                p.tailDir = -1;
                isHeadContract = true;
            } else if(m.dir == 9) { // tail contraction
                particleMap.erase(p.tailPos());
                p.tailDir = -1;
                isHeadContract = false;
            } else {                // invalid contraction
                qDebug() << "invalid contraction index";
                p = before;
                return;
            }
        } else if(p.tailDir == (p.orientation + 3) % 6) {
            if(m.dir == 5) {        // head contraction
                particleMap.erase(p.headPos);
                p.headPos = p.tailPos();
                isHeadContract = true;
                p.tailDir = -1;
            } else if(m.dir == 0) { // tail contraction
                particleMap.erase(p.tailPos());
                p.tailDir = -1;
                isHeadContract = false;
            } else {                // invalid contraction
                qDebug() << "invalid contraction index";
                p = before;
                return;
            }
        } else if(p.tailDir == (p.orientation + 4) % 6) {
            if(m.dir == 6) {        // head contraction
                particleMap.erase(p.headPos);
                p.headPos = p.tailPos();
                p.tailDir = -1;
                isHeadContract = true;
            } else if(m.dir == 1) { // tail contraction
                particleMap.erase(p.tailPos());
                p.tailDir = -1;
                isHeadContract = false;
            } else {                // invalid contraction
                qDebug() << "invalid contraction index";
                p = before;
                return;
            }
        } else if(p.tailDir == (p.orientation + 5) % 6) {
            if(m.dir == 9) {        // head contraction
                particleMap.erase(p.headPos);
                p.headPos = p.tailPos();
                p.tailDir = -1;
                isHeadContract = true;
            } else if(m.dir == 4) { // tail contraction
                particleMap.erase(p.tailPos());
                isHeadContract = false;
                p.tailDir = -1;
            } else {                // invalid contraction
                qDebug() << "invalid contraction index";
                p = before;
                return;
            }
        }

        // look for a particle that wants to join to do a handover
        // if such a particle is found, do a handover
        bool handover = false;
        std::deque<int> neighborDirs = {0, 1, 2, 3, 4, 5};;
        while(neighborDirs.size() > 0) {
            auto dist = std::uniform_int_distribution<int>(0, neighborDirs.size() - 1);
            auto index = dist(rng);
            auto neighborDirIt = neighborDirs.begin() + index;
            auto neighborDir = *neighborDirIt;
            neighborDirs.erase(neighborDirIt);

            Vec neighbor;
            if(isHeadContract) {
                neighbor = before.headPos.vecInDir(neighborDir);
                if(neighbor == before.tailPos()) {
                    continue;
                }
            } else {
                neighbor = before.tailPos().vecInDir(neighborDir);
                if(neighbor == before.headPos) {
                    continue;
                }
            }

            auto mapIt = particleMap.find(neighbor);
            if(mapIt != particleMap.end()) {
                Particle& p2 = mapIt->second;
                if(p2.tailDir != -1) {
                    continue;
                }
                Particle before2 = p2;
                Movement m2 = p2.executeAlgorithm();
                if(m2.type != MovementType::Expand) {
                    p2 = before2;
                    continue;
                }
                int expandDir = posMod(p2.orientation + m2.dir, 6);
                Vec newHeadPos = p2.headPos.vecInDir(expandDir);
                if( (isHeadContract && newHeadPos == before.headPos) ||
                    (!isHeadContract && newHeadPos == before.tailPos())) {
                    // success!
                    particleMap.insert(std::pair<Vec, Particle&>(newHeadPos, p2));
                    p2.headPos = newHeadPos;
                    p2.tailDir = posMod(expandDir + 3, 6);
                    handover = true;
                    break;
                } else {
                    p2 = before2;
                }
            }
        }

        if(m.type == MovementType::HandoverContract && !handover) {
            p = before;
            Vec notFreed;
            if(isHeadContract) {
                notFreed = before.headPos;
            } else {
                notFreed = before.tailPos();
            }
            particleMap.insert(std::pair<Vec, Particle&>(notFreed, p));
        }
    }
}
