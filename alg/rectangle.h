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

    RectangleParticle& nbrAtLabel(int label) const;

    int labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel = 0) const;
    bool hasNeighborInState(std::initializer_list<State> states) const;

    int constructionReceiveDir() const;
    bool canFinish() const;
    int checkFinish();
    void updateConstructionDir();
    void updateMoveDir();
    int countPCwithValue(int value);
    int countPCwithPosValue();
    int getPCValue();
    void setPCValue(int value);
    bool hasTailFollower() const;

protected:
    State state;
    int width = 3;
    int height = 1;
    struct SToken : public Token {
    int value = -1;
    };
    struct PCToken : public Token {
        int value=-1;//width-1
    };
    struct PCPrepToken : public Token {
        bool ontoken = false;
    };
    struct RoundEnd1Token : public Token { };
    struct RoundEnd2Token : public Token { };
    struct RoundEndBToken : public Token { };
    std::shared_ptr<Token> makePCWithValue(int value);

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
    RectangleSystem(int numParticles = 200, double holeProb = 0.2);

    virtual bool hasTerminated() const;
};

#endif // RECTANGLE_H
