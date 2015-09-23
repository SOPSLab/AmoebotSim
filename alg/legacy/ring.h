// Brian Parker
// Note that all of my shape formation algorithms make use of "State" instead of "phase". The two are equivalent in theory, but not in programming.
#ifndef RING_H
#define RING_H

#include "alg/legacy/algorithmwithflags.h"

class LegacySystem;

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
    bool stopper; // This algorithm also makes use of a stopper, which is a "mid-point" for the expanded ring
    bool followIndicator;
    int contractDir;
};

class Ring : public AlgorithmWithFlags<RingFlag>
{
public:
    
    Ring(const State _state);
    Ring(const Ring& other);
    virtual ~Ring();

    static std::shared_ptr<LegacySystem> instance(const unsigned int size, const double holeProb);

    virtual Movement execute();
    virtual std::shared_ptr<Algorithm> blank() const override;
    virtual std::shared_ptr<Algorithm> clone() override;
    virtual bool isDeterministic() const;
    virtual bool isStatic() const;

protected:
    
    
    int isPointedAt(); // 
    void setPoint(int _label);

    // Functions to access the stopper
    bool nearStopper();
    void setStopper(bool value = true);

    void setState(State _state);
    bool neighborInState(int direction, State _state);
    bool hasNeighborInState(State _state);
    int firstNeighborInState(State _state);

    int getMoveDir(); 
    void setContractDir(const int contractDir);
    int updatedFollowDir() const;

    void unsetFollowIndicator();
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
