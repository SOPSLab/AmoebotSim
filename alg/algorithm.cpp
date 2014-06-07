#include "alg/algorithm.h"

const std::vector<int> Algorithm::sixLabels = {{0, 1, 2, 3, 4, 5}};
const std::array<const std::vector<int>, 6> Algorithm::labels =
{{
    {3, 4, 5, 6, 7},
    {4, 5, 6, 7, 8},
    {7, 8, 9, 0, 1},
    {8, 9, 0, 1, 2},
    {9, 0, 1, 2, 3},
    {2, 3, 4, 5, 6}
}};
const std::array<int, 6> Algorithm::contractLabels = {{0, 1, 4, 5, 6, 9}};
const std::array<std::array<int, 10>, 6> Algorithm::labelDir
{{
    {{0, 1, 2, 1, 2, 3, 4, 5, 4, 5}},
    {{0, 1, 2, 3, 2, 3, 4, 5, 0, 5}},
    {{0, 1, 0, 1, 2, 3, 4, 3, 4, 5}},
    {{0, 1, 2, 1, 2, 3, 4, 5, 4, 5}},
    {{0, 1, 2, 3, 2, 3, 4, 5, 0, 5}},
    {{0, 1, 0, 1, 2, 3, 4, 3, 4, 5}}
}};
