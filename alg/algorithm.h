#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <vector>

#include "sim/movement.h"

class Algorithm
{
public:
    Algorithm();
    virtual ~Algorithm();

    virtual Movement execute() = 0;
    virtual Algorithm* clone() = 0;

public:
    bool expanded;

    std::vector<int> headLabels;
    std::vector<int> tailLabels;
};

#endif // ALGORITHM_H
