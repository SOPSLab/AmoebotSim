#ifndef DUMMYALG_H
#define DUMMYALG_H

#include "alg/algorithm.h"

class DummyFlag : public Flag
{
public:
    DummyFlag();
    DummyFlag(const DummyFlag& other);
};

class DummyAlg : public Algorithm
{
public:
    DummyAlg();
    DummyAlg(const DummyAlg& other);
    virtual ~DummyAlg();

    virtual Algorithm* clone();

protected:
    virtual Movement execute(std::array<const Flag*, 10>& flags);
};

#endif // DUMMYALG_H
