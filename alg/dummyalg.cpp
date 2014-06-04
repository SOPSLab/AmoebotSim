#include "alg/dummyalg.h"

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

Movement DummyAlg::execute(std::array<const Flag*, 10>& flags)
{
    auto inFlags = castFlags<DummyFlag>(flags);

    if(expanded) {
        if(inFlags[5] == nullptr) {
            return Movement(MovementType::Contract, tailContractLabel);
        } else {
            return Movement(MovementType::HandoverContract, tailContractLabel);
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

Algorithm* DummyAlg::clone()
{
    return new DummyAlg(*this);
}
