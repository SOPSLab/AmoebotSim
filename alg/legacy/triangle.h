// Brian Parker
// Note that all of my shape formation algorithms make use of "State" instead of "phase". The two are equivalent in theory, but not in programming.
#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "alg/legacy/algorithmwithflags.h"

class LegacySystem;

namespace Triangle
{
enum class State {
    Finished,
    Leader,
    Follower,
    Idle,
    Seed
};

class TriFlag : public Flag
{
public:
    TriFlag();
    TriFlag(const TriFlag& other);
    Triangle::State state;
    bool point; // 
    bool side; // This algorithm makes use of a "side" flag that specifically indicates the right side of the forming triangle
    bool followIndicator;
    int contractDir;
};

class Triangle : public AlgorithmWithFlags<TriFlag>
{
public:
    Triangle(const State _state);
    Triangle(const Triangle& other);
    virtual ~Triangle();

    static std::shared_ptr<LegacySystem> instance(const unsigned int size, const double holeProb);

    virtual Movement execute();
    virtual std::shared_ptr<Algorithm> blank() const override;
    virtual std::shared_ptr<Algorithm> clone() override;
    virtual bool isDeterministic() const;
    virtual bool isStatic() const;

protected:
    int isPointedAt();

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
};
}

#endif // TRIANGLE_H
