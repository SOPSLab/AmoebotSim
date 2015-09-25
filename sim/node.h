#ifndef NODE_H
#define NODE_H

#include <array>

#include <QtGlobal>

class Node
{
public:
    Node();
    Node(int x, int y);
    Node(const Node& other);

    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;

    Node nodeInDir(int dir) const;

public:
    int x, y;
};

bool operator<(const Node& node1, const Node& node2);

inline Node::Node()
    : x(0), y(0)
{

}

inline Node::Node(int x, int y)
    : x(x), y(y)
{

}

inline Node::Node(const Node& other)
    : x(other.x), y(other.y)
{

}

inline bool Node::operator==(const Node& other) const
{
    return (x == other.x) && (y == other.y);
}

inline bool Node::operator!=(const Node& other) const
{
    return !operator==(other);
}

inline Node Node::nodeInDir(int dir) const
{
    Q_ASSERT(0 <= dir && dir <= 5);
    static constexpr std::array<int, 6> xOffset = {{1, 0, -1, -1,  0,  1}};
    static constexpr std::array<int, 6> yOffset = {{0, 1,  1,  0, -1, -1}};
    return Node(x + xOffset[dir], y + yOffset[dir]);
}

inline bool operator<(const Node& v1, const Node& v2)
{
    return (v1.x < v2.x) || (v1.x == v2.x && v1.y < v2.y);
}

#endif // NODE_H
