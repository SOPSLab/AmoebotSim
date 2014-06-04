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

#endif // NODE_H
