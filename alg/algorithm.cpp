#include "alg/algorithm.h"

std::mt19937 Algorithm::rng;
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
const std::array<std::array<int, 10>, 6> Algorithm::labelDir =
{{
    {{0, 1, 2, 1, 2, 3, 4, 5, 4, 5}},
    {{0, 1, 2, 3, 2, 3, 4, 5, 0, 5}},
    {{0, 1, 0, 1, 2, 3, 4, 3, 4, 5}},
    {{0, 1, 2, 1, 2, 3, 4, 5, 4, 5}},
    {{0, 1, 2, 3, 2, 3, 4, 5, 0, 5}},
    {{0, 1, 0, 1, 2, 3, 4, 3, 4, 5}}
}};
const std::array<const std::array<int, 3>, 6> Algorithm::_frontLabels =
{{
    {{4, 5, 6}},
    {{5, 6, 7}},
    {{8, 9, 0}},
    {{9, 0, 1}},
    {{0, 1, 2}},
    {{3, 4, 5}}
}};
const std::array<const std::array<int, 2>, 6> Algorithm::_sideLabelsFromHead =
{{
    {{7, 3}},
    {{8, 4}},
    {{1, 7}},
    {{2, 8}},
    {{3, 9}},
    {{6, 2}}
}};
const std::array<const std::array<int, 2>, 6> Algorithm::_sideLabelsFromTail =
{{
    {{8, 2}},
    {{9, 3}},
    {{2, 6}},
    {{3, 7}},
    {{4, 8}},
    {{1, 7}}
}};
const std::array<const std::array<int, 3>, 6> Algorithm::_backLabels =
{{
    {{9, 0, 1}},
    {{0, 1, 2}},
    {{3, 4, 5}},
    {{4, 5, 6}},
    {{5, 6, 7}},
    {{8, 9, 0}}
}};
