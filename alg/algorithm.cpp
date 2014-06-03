#include "alg/algorithm.h"

Algorithm::Algorithm()
    : expanded(false),
      headLabels({0, 1, 2, 3, 4, 5}),
      tailLabels({})
{
}

Algorithm::Algorithm(const Algorithm& other)
    : expanded(other.expanded),
      headLabels(other.headLabels),
      tailLabels(other.tailLabels)
{
}

Algorithm::~Algorithm()
{
}

Movement Algorithm::delegateExecute()
{
    Movement m = execute();

    if(m.type == MovementType::Expand) {
        expanded = true;

        if(m.dir == 0) {
            headLabels = {8, 9, 0, 1, 2};
            tailLabels = {3, 4, 5, 6, 7};
        } else if(m.dir == 1) {
            headLabels = {9, 0, 1, 2, 3};
            tailLabels = {4, 5, 6, 7, 8};
        } else if(m.dir == 2) {
            headLabels = {2, 3, 4, 5, 6};
            tailLabels = {7, 8, 9, 1, 0};
        } else if(m.dir == 3) {
            headLabels = {3, 4, 5, 6, 7};
            tailLabels = {8, 9, 0, 1, 2};
        } else if(m.dir == 4) {
            headLabels = {4, 5, 6, 7, 8};
            tailLabels = {9, 0, 1, 2, 3};
        } else if(m.dir == 5) {
            headLabels = {7, 8, 9, 0, 1};
            tailLabels = {2, 3, 4, 5, 6};
        }
    }

    if(m.type == MovementType::Contract) {
        expanded = false;

        headLabels = {0, 1, 2, 3, 4, 5};
        tailLabels = {};
    }

    if(m.type == MovementType::HandoverContract) {

    }

    return m;
}

bool Algorithm::isExpanded() const
{
    return expanded;
}

std::vector<int> Algorithm::getHeadLabels() const
{
    return headLabels;
}

std::vector<int> Algorithm::getTailLabels() const
{
    return tailLabels;
}
