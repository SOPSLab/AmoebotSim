#include "vec.h"

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
