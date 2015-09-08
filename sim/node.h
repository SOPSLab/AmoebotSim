#ifndef NODE_H
#define NODE_H

class Node
{
public:
    Node();
    Node(const int _x, const int _y);
    Node(const Node& other);

    bool operator==(const Node& other);
    bool operator!=(const Node& other);

    Node nodeInDir(int dir) const;

public:
    int x, y;
};

bool operator<(const Node& v1, const Node& v2);

inline Node::Node()
    : x(0), y(0)
{
}

inline Node::Node(const int _x, const int _y) :
    x(_x),
    y(_y)
{
}

inline Node::Node(const Node& other) :
    x(other.x),
    y(other.y)
{
}

inline bool Node::operator==(const Node& other)
{
    return (x == other.x) && (y == other.y);
}

inline bool Node::operator!=(const Node& other)
{
    return !operator==(other);
}

inline Node Node::nodeInDir(int dir) const
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

inline bool operator<(const Node& v1, const Node& v2)
{
    return (v1.x < v2.x) || (v1.x == v2.x && v1.y < v2.y);
}

#endif // NODE_H
