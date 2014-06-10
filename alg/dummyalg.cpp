#include "alg/dummyalg.h"
#include "sim/particle.h"
#include "sim/system.h"

DummyFlag::DummyFlag()
{
}

DummyFlag::DummyFlag(const DummyFlag& other)
    : Flag(other)
{
}

DummyAlg::DummyAlg()
{
    initFlags<DummyFlag>();
}

DummyAlg::DummyAlg(const DummyAlg& other)
    : Algorithm(other)
{
    copyFlags<DummyFlag>(other);
}

DummyAlg::~DummyAlg()
{
    deleteFlags();
}

System DummyAlg::instance()
{
    System system;
    for(int x = 0; x < 6; x++) {
        system.insert(Particle(new DummyAlg(), 0, Node(x, 0), -1));
    }
    return system;
}

Algorithm* DummyAlg::clone()
{
    return new DummyAlg(*this);
}

Movement DummyAlg::execute(std::array<const Flag*, 10>& flags)
{
    auto inFlags = castFlags<DummyFlag>(flags);

    if(isExpanded()) {
        if(inFlags[5] == nullptr) {
            return Movement(MovementType::Contract, tailContractionLabel());
        } else {
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        }
    } else {
        if(inFlags[0] == nullptr) {
            headColor = 0xff0000;
            tailColor = 0xff0000;
        } else if(inFlags[3] == nullptr) {
            headColor = 0x00ff00;
            tailColor = 0x00ff00;
        } else {
            headColor = 0x0000ff;
            tailColor = 0x0000ff;
        }
        return Movement(MovementType::Expand, 0);
    }
}
