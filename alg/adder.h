#ifndef ADDER_H
#define ADDER_H

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class AdderParticle : public AmoebotParticle
{
    friend class AdderSystem;

public:
    enum class State
    {
        Seed,
        Idle,
        Follow,
        Lead,
        Finish,
        Active
    };

    AdderParticle(const Node head,
                    const int globalTailDir,
                    const int orientation,
                    AmoebotSystem& system,
                    State state);

    virtual void activate();

    virtual int headMarkColor() const;
    virtual int headMarkDir() const;
    virtual int tailMarkColor() const;

    virtual QString inspectionText() const;

    AdderParticle& nbrAtLabel(int label) const;

    int labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel = 0) const;
    bool hasNeighborInState(std::initializer_list<State> states) const;

    int constructionReceiveDir() const;
    bool canFinish() const;
    void updateConstructionDir();

    void updateMoveDir();

    bool hasTailFollower() const;
    bool hasSpace();
    void setCounterGoal(int goal);

protected:
    struct CarryToken : public Token { };

    State state;

    int constructionDir;
    int moveDir;
    int followDir;
    int prevLabel = -1;
    int nextLabel = -1;
    int index;
    int counterGoal = 1;
    int displayVal=0;

};

class AdderSystem : public AmoebotSystem
{
public:
    AdderSystem(int numParticles = 10, int countValue = 250);

    virtual bool hasTerminated() const;
};

#endif // ADDER_h
