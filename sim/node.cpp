#include <QtGlobal>

#include "sim/node.h"

Node::Node()
{
}

Node::Node(const int _x, const int _y) :
    x(_x),
    y(_y)
{
}

Node::Node(const Node& other) :
    x(other.x),
    y(other.y)
{
}

bool Node::operator==(const Node& other)
{
    return (x == other.x) && (y == other.y);
}

bool Node::operator!=(const Node& other)
{
    return !operator==(other);
}

Node Node::nodeInDir(int dir) const
{
    Q_ASSERT(0 <= dir && dir <= 5);
    if(dir == 0) {
        return Node(x + 1, y + 0);
    } else if(dir == 1) {
        return Node(x + 0, y + 1);
    } else if(dir == 2) {
        return Node(x - 1, y + 1);
    } else if(dir == 3) {
        return Node(x - 1, y + 0);
    } else if(dir == 4) {
        return Node(x + 0, y - 1);
    } else { // dir == 5
        return Node(x + 1, y - 1);
    }
}

bool operator<(const Node& v1, const Node& v2)
{
    return (v1.x < v2.x) || (v1.x == v2.x && v1.y < v2.y);
}
