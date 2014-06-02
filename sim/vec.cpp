#include "vec.h"

#include <QtGlobal>

Vec::Vec()
{
}

Vec::Vec(const int _x, const int _y) :
    x(_x),
    y(_y)
{
}

Vec::Vec(const Vec& other) :
    x(other.x),
    y(other.y)
{
}

bool Vec::operator==(const Vec& other)
{
    return (x == other.x) && (y == other.y);
}

bool Vec::operator!=(const Vec& other)
{
    return !operator==(other);
}

Vec Vec::vecInDir(int dir) const
{
    Q_ASSERT(0 <= dir && dir <= 5);
    if(dir == 0) {
        return Vec(x + 1, y + 0);
    }
    if(dir == 1) {
        return Vec(x + 0, y + 1);
    }
    if(dir == 2) {
        return Vec(x - 1, y + 1);
    }
    if(dir == 3) {
        return Vec(x - 1, y + 0);
    }
    if(dir == 4) {
        return Vec(x + 0, y - 1);
    }
    if(dir == 5) {
        return Vec(x + 1, y - 1);
    }
    return *this;
}
