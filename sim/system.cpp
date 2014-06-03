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

    if(m.type == MovementType::Idle) {

    } else if(m.type == MovementType::Expand) {
        if(p.tailDir != -1) {
            qDebug() << "already expanded particle cannot expand";
            p = backup;
        } else if(0 <= m.dir && m.dir <= 5) {
            int dir = posMod(p.orientation + m.dir, 6);
            p.headPos = p.headPos.vecInDir(dir);
            p.tailDir = posMod(dir + 3, 6);
        } else {
            qDebug() << "invalid expansion index";
            p = backup;
        }
    } else if(m.type == MovementType::Contract) {
        if(p.tailDir == -1) {
            qDebug() << "already contracted particle cannot contract";
            p = backup;
        } else if(p.tailDir == p.orientation) {
            if(m.dir == 0) {        // head contraction
                p.headPos = p.headPos.vecInDir(p.tailDir);
                p.tailDir = -1;
            } else if(m.dir == 5) { // tail contraction
                p.tailDir = -1;
            } else {                // invalid contraction
                qDebug() << "invalid contraction index";
                p = backup;
            }
        } else if(p.tailDir == (p.orientation + 1) % 6) {
            if(m.dir == 1) {        // head contraction
                p.headPos = p.headPos.vecInDir(p.tailDir);
                p.tailDir = -1;
            } else if(m.dir == 6) { // tail contraction
                p.tailDir = -1;
            } else {                // invalid contraction
                qDebug() << "invalid contraction index";
                p = backup;
            }
        } else if(p.tailDir == (p.orientation + 2) % 6) {
            if(m.dir == 4) {        // head contraction
                p.headPos = p.headPos.vecInDir(p.tailDir);
                p.tailDir = -1;
            } else if(m.dir == 9) { // tail contraction
                p.tailDir = -1;
            } else {                // invalid contraction
                qDebug() << "invalid contraction index";
                p = backup;
            }
        } else if(p.tailDir == (p.orientation + 3) % 6) {
            if(m.dir == 5) {        // head contraction
                p.headPos = p.headPos.vecInDir(p.tailDir);
                p.tailDir = -1;
            } else if(m.dir == 0) { // tail contraction
                p.tailDir = -1;
            } else {                // invalid contraction
                qDebug() << "invalid contraction index";
                p = backup;
            }
        } else if(p.tailDir == (p.orientation + 4) % 6) {
            if(m.dir == 6) {        // head contraction
                p.headPos = p.headPos.vecInDir(p.tailDir);
                p.tailDir = -1;
            } else if(m.dir == 1) { // tail contraction
                p.tailDir = -1;
            } else {                // invalid contraction
                qDebug() << "invalid contraction index";
                p = backup;
            }
        } else if(p.tailDir == (p.orientation + 5) % 6) {
            if(m.dir == 9) {        // head contraction
                p.headPos = p.headPos.vecInDir(p.tailDir);
                p.tailDir = -1;
            } else if(m.dir == 4) { // tail contraction
                p.tailDir = -1;
            } else {                // invalid contraction
                qDebug() << "invalid contraction index";
                p = backup;
            }
        }
    } else if(m.type == MovementType::HandoverContract) {

    }
}
