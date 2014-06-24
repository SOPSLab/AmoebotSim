#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "alg/algorithm.h"
class TriFlag;
class System;

class Triangle : public Algorithm
{
public:
    enum class State {
        Finished,
        Leader,
        Follower,
        Idle,
        Seed
    };
    Triangle(const State _state);
    Triangle(const Triangle& other);
    virtual ~Triangle();
    static System* instance(const int size, const double holeProb);
    virtual Algorithm* clone();
    virtual bool isDeterministic() const;

protected:
    virtual Movement execute(std::array<const Flag*, 10>& flags);
    
    int isPointedAt();

    void setState(State _state);
    bool neighborInState(int direction, State _state);
    bool hasNeighborInState(State _state);
    int firstNeighborInState(State _state);

    int getMoveDir(); 
    void setContractDir(const int contractDir);
    int updatedFollowDir() const;

    int unsetFollowIndicator() const;
    void setFollowIndicatorLabel(const int label);
    bool tailReceivesFollowIndicator() const;
    bool followIndicatorMatchState(State _state) const;
    
protected:
    std::array<const TriFlag*, 10> inFlags;
    std::array<TriFlag*, 10> outFlags;
    State state;
    int followDir;
};

class TriFlag : public Flag
{
public:
    TriFlag();
    TriFlag(const TriFlag& other);
    Triangle::State state;
    bool point;
    bool side;
    bool followIndicator;
    int contractDir;
};

#endif // TRIANGLE_H
