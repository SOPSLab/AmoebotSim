#ifndef MISC
#define MISC

#include <functional>
#include <set>

#include "core/node.h"

// given a set of nodes, this function returns surrounding set of nodes for which checkFunc return true
// this is a nice tool to grow layers from a set of nodes (hence the name)
inline std::set<Node> nextLayer(const std::set<Node>& lastLayer, std::function<bool(Node)> checkFunc)
{
    std::set<Node> layer;
    for(const Node& node : lastLayer) {
        for(int i = 0; i < 6; i++) {
            const Node neighbor = node.nodeInDir(i);
            if(checkFunc(neighbor)) {
                layer.insert(neighbor);
            }
        }
    }
    return layer;
}

// BEWARE: This is a quite special case of a binary search right now. It should become more general later on.
inline int binarySearch(const int start, std::function<bool(int)> func)
{
    Q_ASSERT(start > 0);

    if(func(0)) {
        return 0;
    }

    int value = start;
    int delta = start;
    while(delta >= 1) {
        if(func(value)) {
            value = value - delta;
        } else {
            value = value + delta;
        }
        delta = delta / 2;
    }

    if(!func(value)) {
        value = value + 1;
    }

    Q_ASSERT(func(value) && !func(value - 1));
    return value;
}

#endif // MISC

