#include "alg/dummyalg.h"

DummyAlg::DummyAlg()
    : expanded(false)
{
}

DummyAlg::DummyAlg(const DummyAlg& other)
    : expanded(other.expanded)
{
}

DummyAlg::~DummyAlg()
{
}

Movement DummyAlg::execute()
{
    if(expanded) {
        expanded = false;
        return Movement(MovementType::Contract, 0);
    } else {
        expanded = true;
        return Movement(MovementType::Expand, 0);
    }
}

Algorithm* DummyAlg::clone()
{
    return new DummyAlg(*this);
}
