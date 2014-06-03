#include <chrono>

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
    Movement m = p.executeAlgorithm();

    if(m.type == MovementType::Idle) {
        return;
    }

    if(m.type == MovementType::Expand) {
        int dir = posMod(p.orientation + m.dir, 6);
        p.headPos = p.headPos.vecInDir(dir);
        p.tailDir = posMod(dir + 3, 6);
        return;
    }

    if(m.type == MovementType::Contract) {
        p.tailDir = -1;
        return;
    }

    if(m.type == MovementType::HandoverContract) {

    }
}
