#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "sim/movement.h"

class Algorithm
{
public:
    Algorithm();
    virtual ~Algorithm();

    virtual Movement execute() = 0;
    virtual Algorithm* clone() = 0;
};

#endif // ALGORITHM_H
