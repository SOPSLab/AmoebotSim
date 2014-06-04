#include "alg/algorithm.h"

Flag::Flag()
{
}

Flag::Flag(const Flag& other)
    : expanded(other.expanded)
{
}

Algorithm::Algorithm()
    : expanded(false),
      headLabels({0, 1, 2, 3, 4, 5}),
      tailLabels({0, 1, 2, 3, 4, 5}),
      headContractLabel(-1),
      tailContractLabel(-1)
{
}

Algorithm::Algorithm(const Algorithm& other)
    : expanded(other.expanded),
      headLabels(other.headLabels),
      tailLabels(other.tailLabels),
      headContractLabel(other.headContractLabel),
      tailContractLabel(other.tailContractLabel)
{
}

Algorithm::~Algorithm()
{
}

Movement Algorithm::delegateExecute(std::array<const Flag*, 10>& flags)
{
    Movement m = execute(flags);

    if(m.type == MovementType::Expand) {
        expanded = true;

        if(m.dir == 0) {
            headLabels = {8, 9, 0, 1, 2};
            tailLabels = {3, 4, 5, 6, 7};
            headContractLabel = 5;
            tailContractLabel = 0;
        } else if(m.dir == 1) {
            headLabels = {9, 0, 1, 2, 3};
            tailLabels = {4, 5, 6, 7, 8};
            headContractLabel = 6;
            tailContractLabel = 1;
        } else if(m.dir == 2) {
            headLabels = {2, 3, 4, 5, 6};
            tailLabels = {7, 8, 9, 0, 1};
            headContractLabel = 9;
            tailContractLabel = 4;
        } else if(m.dir == 3) {
            headLabels = {3, 4, 5, 6, 7};
            tailLabels = {8, 9, 0, 1, 2};
            headContractLabel = 0;
            tailContractLabel = 5;
        } else if(m.dir == 4) {
            headLabels = {4, 5, 6, 7, 8};
            tailLabels = {9, 0, 1, 2, 3};
            headContractLabel = 1;
            tailContractLabel = 6;
        } else if(m.dir == 5) {
            headLabels = {7, 8, 9, 0, 1};
            tailLabels = {2, 3, 4, 5, 6};
            headContractLabel = 4;
            tailContractLabel = 9;
        }
    } else if(m.type == MovementType::Contract || m.type == MovementType::HandoverContract) {
        expanded = false;
        headLabels = {0, 1, 2, 3, 4, 5};
        tailLabels = {0, 1, 2, 3, 4, 5};
        headContractLabel = -1;
        tailContractLabel = -1;
    }

    for(auto it = outFlags.begin(); it != outFlags.end(); ++it) {
        Flag* flag = *it;
        flag->expanded = expanded;
    }

    return m;
}
