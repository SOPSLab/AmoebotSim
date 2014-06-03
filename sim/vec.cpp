#include <QtGlobal>

#include "sim/vec.h"

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
    Q_ASSERT(-1 <= dir && dir <= 5);
    if(dir == -1) {
        return *this;
    } else if(dir == 0) {
        return Vec(x + 1, y + 0);
    } else if(dir == 1) {
        return Vec(x + 0, y + 1);
    } else if(dir == 2) {
        return Vec(x - 1, y + 1);
    } else if(dir == 3) {
        return Vec(x - 1, y + 0);
    } else if(dir == 4) {
        return Vec(x + 0, y - 1);
    } else if(dir == 5) {
        return Vec(x + 1, y - 1);
    }
    return *this;
}

bool operator<(const Vec& v1, const Vec& v2)
{
    return (v1.x < v2.x) || (v1.x == v2.x && v1.y < v2.y);
}
