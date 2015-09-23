#ifndef HOLEELIMSTANDARD_H
#define HOLEELIMSTANDARD_H

#include "alg/legacy/algorithmwithflags.h"

class LegacySystem;

namespace HoleElimStandard
{

enum class State {
    Seed,
    Idle,
    Walking,
    Follower,
    Finished
};

class HoleElimStandardFlag : public Flag
{
public:
    HoleElimStandardFlag();
    HoleElimStandardFlag(const HoleElimStandardFlag& other);

    State state;
    bool isParent;
    bool dockingIndicator;
};

class HoleElimStandard : public AlgorithmWithFlags<HoleElimStandardFlag>
{
public:
    HoleElimStandard(const State _state);
    HoleElimStandard(const HoleElimStandard& other);
    virtual ~HoleElimStandard();

    static std::shared_ptr<LegacySystem> instance(const unsigned int size);

    virtual Movement execute();

    virtual std::shared_ptr<Algorithm> blank() const override;
    virtual std::shared_ptr<Algorithm> clone() override;

    virtual bool isDeterministic() const;
    virtual bool isStatic() const;

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

#endif // HOLEELIMSTANDARD_H
