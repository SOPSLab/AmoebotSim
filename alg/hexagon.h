#ifndef HEXAGON_H
#define HEXAGON_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class HexagonParticle : public AmoebotParticle
{
    friend class HexagonSystem;

public:
    enum class State
    {
        Seed,
        Idle,
        Follow,
        Lead,
        Finish
    };

public:
    HexagonParticle(const Node head,
                    const int globalTailDir,
                    const int orientation,
                    std::map<Node, AmoebotParticle*>& particleMap,
                    State state);

    virtual void activate();

    virtual int headMarkColor() const;
    virtual int headMarkDir() const;
    virtual int tailMarkColor() const;

    virtual HexagonParticle& neighborAtLabel(int label) const;

    int labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel = 0) const;
    bool hasNeighborInState(std::initializer_list<State> states) const;

    int constructionReceiveDir() const;
    bool canFinish() const;
    void updateConstructionDir();

    void updateMoveDir();

    bool hasTailFollower() const;

protected:
    State state;

    int constructionDir;
    int moveDir;
    int followDir;
};

class HexagonSystem : public AmoebotSystem
{
public:
    HexagonSystem(int numParticles = 200, float holeProb = 0.2);

    virtual bool hasTerminated() const;
};

#endif // HEXAGON_H
