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
    Movement(MovementType _type, int _label = 0);

public:
    MovementType type;
    int label;
};

inline Movement::Movement(MovementType _type, int _label) :
    type(_type),
    label(_label)
{
}

#endif // MOVEMENT_H
