#ifndef HEXAGON_H
#define HEXAGON_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class HexagonParticle : public AmoebotParticle
{
    friend class HexagonSystem;

protected:
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
    virtual int headMarkGlobalDir() const;
    virtual int tailMarkColor() const;

    virtual HexagonParticle& neighborAtLabel(int label) const;

    int labelOfFirstNeighborInState(std::initializer_list<State> states) const;
    bool hasNeighborInState(std::initializer_list<State> states) const;

    int constructionReceiveDir() const;
    bool canFinish() const;
    void updateConstructionDir();

    void updateMoveDir();

    bool hasTailFollower() const;

private:
    State state;

    int constructionDir;
    int moveDir;
    int followDir;
};

class HexagonSystem : public AmoebotSystem
{
public:
    HexagonSystem();

    virtual bool hasTerminated() const;
};

#endif // HEXAGON_H
