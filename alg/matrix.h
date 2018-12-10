#ifndef MATRIX_H
#define MATRIX_H

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class MatrixParticle : public AmoebotParticle
{
    friend class MatrixSystem;

public:
    enum class State
    {
        Seed,
        Idle,
        Follow,
        Lead,
        Finish,
        Active,
        Matrix,
        Vector,
        Result

    };

    MatrixParticle(const Node head,
                    const int globalTailDir,
                    const int orientation,
                    AmoebotSystem& system,
                    State state);

    virtual void activate();

    virtual int headMarkColor() const;
    virtual int headMarkDir() const;
    virtual int tailMarkColor() const;

    virtual QString inspectionText() const;

    MatrixParticle& nbrAtLabel(int label) const;

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
    struct VectorToken : public Token {
        int value = -1;
    };

    struct ProductToken : public Token {
        int value = -1;
    };


    State state;

    int constructionDir;
    int moveDir;
    int followDir;
    int prevLabel = -1;
    int nextLabel = -1;
    int index;
    int counterGoal = 1;
    int displayVal=0;
    int locationValue = 0;
    bool setlocValue  =false;
    int resultValue= -1;
    const int MaxValue = 100;
    bool sentProduct = false;

};

class MatrixSystem : public AmoebotSystem
{
public:
    MatrixSystem(int numParticles = 10, int countValue = 250);

    virtual bool hasTerminated() const;
};

#endif // Matrix_h
