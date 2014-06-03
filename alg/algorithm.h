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
    const std::vector<int>& getHeadLabels() const;
    const std::vector<int>& getTailLabels() const;
    int getHeadContractLabel() const;
    int getTailContractLabel() const;

private:
    bool expanded;
    std::vector<int> headLabels;
    std::vector<int> tailLabels;
    int headContractLabel;
    int tailContractLabel;
};

#endif // ALGORITHM_H
