#ifndef HOLEELIMCOATING_H
#define HOLEELIMCOATING_H

#include "alg/algorithmwithflags.h"

class System;

namespace HoleElimCoating
{

enum class State {
    Seed,
    Idle,
    Walking,
    Follower,
    Finished
};

class HoleElimCoatingFlag : public Flag
{
public:
    HoleElimCoatingFlag();
    HoleElimCoatingFlag(const HoleElimCoatingFlag& other);

    State state;
    bool isParent;
    bool dockingIndicator;
};

class HoleElimCoating : public AlgorithmWithFlags<HoleElimCoatingFlag>
{
public:
    HoleElimCoating(const State _state);
    HoleElimCoating(const HoleElimCoating& other);
    virtual ~HoleElimCoating();

    static std::shared_ptr<System> instance(const unsigned int size, const double holeProb);

    virtual Movement execute();

    virtual std::shared_ptr<Algorithm> clone() override;

    virtual bool isDeterministic() const;

protected:
    void setState(const State _state);
    void setParentLabel(const int parentLabel);
    void setDockingLabel(const int dockingLabel);

    bool hasNeighborInState(const State _state) const;
    int neighborInStateDir(const State _state) const;
    int dockingDir() const;
    int adjFinishDir() const;
    int borderFinishedDir() const;
    bool tailHasChild() const;

    State state;
    int followDir;
};

}

#endif // HOLEELIMCOATING_H
