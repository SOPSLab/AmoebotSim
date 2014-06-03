#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <vector>

#include "sim/movement.h"

class Algorithm
{
public:
    Algorithm();
    Algorithm(const Algorithm& other);
    virtual ~Algorithm();

    Movement delegateExecute();

    virtual Movement execute() = 0;
    virtual Algorithm* clone() = 0;

    bool isExpanded() const;
    std::vector<int> getHeadLabels() const;
    std::vector<int> getTailLabels() const;

private:
    bool expanded;

    std::vector<int> headLabels;
    std::vector<int> tailLabels;
};

#endif // ALGORITHM_H
