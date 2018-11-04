#ifndef SIERPINSKI_H
#define SIERPINSKI_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class SierpinskiParticle : public AmoebotParticle
{
    friend class SierpinskiSystem;

public:
    enum class State
    {
        Seed,
        Idle,
        Follow,
        Lead,
        Finish,
        Wait
    };

public:
    SierpinskiParticle(const Node head,
                    const int globalTailDir,
                    const int orientation,
                    AmoebotSystem& system,
                    State state);

    virtual void activate();

    virtual int headMarkColor() const;
    virtual int headMarkDir() const;
    virtual int tailMarkColor() const;

    virtual QString inspectionText() const;

    SierpinskiParticle& nbrAtLabel(int label) const;

    int labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel = 0) const;
    bool hasNeighborInState(std::initializer_list<State> states) const;

    int constructionReceiveDir() const;
    int sideReceiveDir() const;

    bool canFinish() const;
    void updateConstructionDir();

    void updateMoveDir();

    bool hasTailFollower() const;
    int numFractalPointers() const;
    int fractalPoint1;
    int fractalPoint2;
protected:
    State state;

    int constructionDir;
    int constructionDir2;
    int sideDir;
    int moveDir;
    int followDir;

};

class SierpinskiSystem : public AmoebotSystem
{
public:
    SierpinskiSystem(int numParticles = 200, double holeProb = 0.2);

    virtual bool hasTerminated() const;
};

#endif // SIERPINSKI_H
