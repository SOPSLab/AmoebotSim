// Brian Parker
// Note that all of my shape formation algorithms make use of "State" instead of "phase". The two are equivalent in theory, but not in programming.
#ifndef HEXAGON_H
#define HEXAGON_H

#include "alg/algorithmwithflags.h"

class System;

namespace Hexagon
{
enum class State {
    Finished,
    Leader,
    Follower,
    Idle,
    Seed
};

// The pointer is initialized here in the HexFlag class.note the use of namespaces and classnames throughout.
class HexFlag : public Flag
{
public:
    HexFlag();
    HexFlag(const HexFlag& other);
    State state;
    bool point; // 
    bool followIndicator;
    int contractDir;
};

class Hexagon : public AlgorithmWithFlags<HexFlag>
{
public:
    Hexagon(const State _state);
    Hexagon(const Hexagon& other);
    virtual ~Hexagon();

    static std::shared_ptr<System> instance(const unsigned int size, const double holeProb);

    virtual Movement execute();
    virtual std::shared_ptr<Algorithm> blank() const override;
    virtual std::shared_ptr<Algorithm> clone() override;
    virtual bool isDeterministic() const;
    virtual bool isStatic() const;

protected:
    int isPointedAt(); // Function to easily use the pointer mechanic

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

#endif // HEXAGON_H

