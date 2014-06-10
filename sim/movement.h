#ifndef MOVEMENT_H
#define MOVEMENT_H

enum class MovementType
{
    Empty,
    Idle,
    Expand,
    Contract,
    HandoverContract
};

class Movement
{
public:
    Movement(MovementType _type, int _dir = 0);

public:
    MovementType type;
    int dir;
};

inline Movement::Movement(MovementType _type, int _dir) :
    type(_type),
    dir(_dir)
{
}

#endif // MOVEMENT_H
