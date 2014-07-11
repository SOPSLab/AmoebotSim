#ifndef RING_H
#define RING_H

#include "alg/algorithmwithflags.h"

class System;

namespace Ring
{
enum class State {
    Finished,
    Leader2,
    Follower2,
    Set,
    Leader,
    Follower,
    Idle,
    Seed
};


class RingFlag : public Flag
{
public:
    RingFlag();
    RingFlag(const RingFlag& other);
    Ring::State state;
    bool point; //
    bool stopper;
    bool followIndicator;
    int contractDir;
};

class Ring : public AlgorithmWithFlags<RingFlag>
{
public:
    
    Ring(const State _state);
    Ring(const Ring& other);
    virtual ~Ring();

    static System* instance(const unsigned int size, const double holeProb);

    virtual Movement execute();
    virtual Algorithm* clone();
    virtual bool isDeterministic() const;

protected:
    
    
    int isPointedAt(); // 
    void setPoint(int _label);

    bool nearStopper();
    void setStopper(bool value = true);

    void setState(State _state);
    bool neighborInState(int direction, State _state);
    bool hasNeighborInState(State _state);
    int firstNeighborInState(State _state);

    int getMoveDir(); 
    void setContractDir(const int contractDir);
    int updatedFollowDir() const;

    int unsetFollowIndicator();
    void setFollowIndicatorLabel(const int label);
    bool tailReceivesFollowIndicator() const;
    bool followIndicatorMatchState(State _state) const;
    
protected:
    State state;
    int followDir;
    int wait;
};
}



#endif // RING_H
