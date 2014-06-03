#ifndef DUMMYALG_H
#define DUMMYALG_H

#include "alg/algorithm.h"

class DummyAlg : public Algorithm
{
public:
    DummyAlg();
    DummyAlg(const DummyAlg& other);
    virtual ~DummyAlg();

    virtual Movement execute();
    virtual Algorithm* clone();

    int expandDir;
};

#endif // DUMMYALG_H
