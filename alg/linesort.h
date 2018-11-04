#ifndef LINESORT_H
#define LINESORT_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class LineSortParticle : public AmoebotParticle
{
    friend class LineSortSystem;

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
    LineSortParticle(const Node head,
                    const int globalTailDir,
                    const int orientation,
                    AmoebotSystem& system,
                    State state);

    virtual void activate();

    virtual int headMarkColor() const;
    virtual int headMarkDir() const;
     int valueToHex(int colorValue) const;

    virtual int tailMarkColor() const;

    virtual QString inspectionText() const;

    LineSortParticle& nbrAtLabel(int label) const;

    int labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel = 0) const;
    bool hasNeighborInState(std::initializer_list<State> states) const;

    int constructionReceiveDir1() const;
    int constructionReceiveDir2() const;

    bool canWait() const;
    bool canInsert() ;
    int findExpandedWaitNeighbor();
    bool insertsAvailable(int expandedWaitDir);

    void updateConstructionDir();

    void updateMoveDir();

    bool hasTailFollower() const;

protected:
    State state;

    int constructionDir;
    int constructionDir2;
    int sideDir;
    int moveDir;
    int followDir;
    int value;
    int maxValue = 100;
    int insertDir;

    struct ComplaintToken : public Token {    };



};

class LineSortSystem : public AmoebotSystem
{
public:
    LineSortSystem(int numParticles = 200, double holeProb = 0.2);

    virtual bool hasTerminated() const;
};

#endif // LINESORT_H
