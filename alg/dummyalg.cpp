#include "alg/dummyalg.h"

DummyAlg::DummyAlg()
    : expandDir(0)
{
}

DummyAlg::DummyAlg(const DummyAlg& other)
    : Algorithm(other),
      expandDir(other.expandDir)
{
}

DummyAlg::~DummyAlg()
{
}

Movement DummyAlg::execute()
{
    if(isExpanded()) {
        return Movement(MovementType::Contract, getTailContractLabel());
    } else {
        expandDir = (expandDir + 1) % 6;
        return Movement(MovementType::Expand, expandDir);
    }
}

Algorithm* DummyAlg::clone()
{
    return new DummyAlg(*this);
}
