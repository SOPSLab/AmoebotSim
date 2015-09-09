#ifndef RECT
#define RECT

#include <numeric>

class Rect
{
public:
    inline Rect() :
        minX(std::numeric_limits<int>::max()),
        maxX(std::numeric_limits<int>::min()),
        minY(std::numeric_limits<int>::max()),
        maxY(std::numeric_limits<int>::min())
    { }

    inline void include(const Node& node)
    {
        if(node.x < minX) minX = node.x;
        if(node.x > maxX) maxX = node.x;
        if(node.y < minY) minY = node.y;
        if(node.y > maxY) maxY = node.y;
    }

    inline bool contains(const Node& node) const
    {
        return minX <= node.x && node.x <= maxX &&
               minY <= node.y && node.y <= maxY;
    }

    int minX, maxX, minY, maxY;
};

#endif // RECT

