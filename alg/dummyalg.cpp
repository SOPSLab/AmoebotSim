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
        return Movement(MovementType::Expand, 0);
    }
}

Algorithm* DummyAlg::clone()
{
    return new DummyAlg(*this);
}
