#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class RectangleParticle : public AmoebotParticle
{
    friend class RectangleSystem;

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
    RectangleParticle(const Node head,
                    const int globalTailDir,
                    const int orientation,
                    AmoebotSystem& system,
                    State state);

    virtual void activate();

    virtual int headMarkColor() const;
    virtual int headMarkDir() const;
    virtual int tailMarkColor() const;

    virtual QString inspectionText() const;

    RectangleParticle& neighborAtLabel(int label) const;

    int labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel = 0) const;
    bool hasNeighborInState(std::initializer_list<State> states) const;

    int constructionReceiveDir() const;
    bool canFinish() const;
    int checkFinish();
    void updateConstructionDir();
    void updateMoveDir();

    bool hasTailFollower() const;

protected:
    State state;
    struct SToken : public Token { };
    struct PC0Token : public Token { };
    struct PC1Token : public Token { };
    struct PCPrepToken : public Token { };
    struct RoundEnd1Token : public Token { };
    struct RoundEnd2Token : public Token { };

    int buildDir;
    int fillDir1;
    int fillDir2;
    int buildDirReverse;
    int constructionDir;
    int moveDir;
    bool hasSentPC;
    bool hasConsumedPC;
    int followDir;
};

class RectangleSystem : public AmoebotSystem
{
public:
    RectangleSystem(int numParticles = 200, float holeProb = 0.2);

    virtual bool hasTerminated() const;
};

#endif // RECTANGLE_H
