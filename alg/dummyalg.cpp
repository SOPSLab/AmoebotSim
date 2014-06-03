#include "alg/dummyalg.h"

DummyAlg::DummyAlg()
{
}

DummyAlg::DummyAlg(const DummyAlg& other)
    : Algorithm(other)
{
}

DummyAlg::~DummyAlg()
{
}

Movement DummyAlg::execute()
{
    if(isExpanded()) {
        return Movement(MovementType::HandoverContract, getTailContractLabel());
    } else {
        return Movement(MovementType::Expand, 0);
    }
}

Algorithm* DummyAlg::clone()
{
    return new DummyAlg(*this);
}
