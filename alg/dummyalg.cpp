#include "alg/dummyalg.h"

DummyAlg::DummyAlg()
{
}

DummyAlg::DummyAlg(const DummyAlg& other)
{
}

DummyAlg::~DummyAlg()
{
}

Movement DummyAlg::execute()
{
    if(expanded) {
        return Movement(MovementType::Contract, 0);
    } else {
        return Movement(MovementType::Expand, 0);
    }
}

Algorithm* DummyAlg::clone()
{
    return new DummyAlg(*this);
}
