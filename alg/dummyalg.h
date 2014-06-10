#ifndef DUMMYALG_H
#define DUMMYALG_H

#include <QString>

#include "alg/algorithm.h"
#include "sim/system.h"

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

    static System instance();
    virtual Algorithm* clone();
    virtual bool isDeterministic() const;

protected:
    virtual Movement execute(std::array<const Flag*, 10>& flags);
};

#endif // DUMMYALG_H
