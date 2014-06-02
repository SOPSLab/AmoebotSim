#ifndef VEC_H
#define VEC_H

class Vec
{
public:
    Vec();
    Vec(const int _x, const int _y);
    Vec(const Vec& other);

    bool operator==(const Vec& other);
    bool operator!=(const Vec& other);

    Vec vecInDir(int dir) const;

public:
    int x, y;
};

#endif // VEC_H
