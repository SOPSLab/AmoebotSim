#ifndef MATRIX2_H
#define MATRIX2_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"
#include <iostream>
#include <fstream>
#include <string>
class Matrix2Particle : public AmoebotParticle
{
    friend class Matrix2System;

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

    Matrix2Particle(const Node head,
                    const int globalTailDir,
                    const int orientation,
                    AmoebotSystem& system,
                    State state);

    virtual void activate();

    virtual int headMarkColor() const;
    virtual int headMarkDir() const;
    virtual int tailMarkColor() const;

    virtual QString inspectionText() const;

    Matrix2Particle& neighborAtLabel(int label) const;

    int labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel = 0) const;
    bool hasNeighborInState(std::initializer_list<State> states) const;

    int constructionReceiveDir() const;
    bool canFinish() const;
    int tryVectorStop() const;
    int tryMatrixStop() const;
    int tryResultStop() const;
    bool noTokensAtLabel(int label);
    void updateConstructionDir();

    void updateMoveDir();

    bool hasTailFollower() const;
    bool hasSpace();
    void setCounterGoal(int goal);

protected:
    struct VectorToken : public Token {
        int value = -1;
    };
    struct MatrixToken : public Token {
        int value = -1;
    };
    struct EndOfColumnToken : public Token {};
    struct EndOfVectorToken : public Token {};
    struct StartMultToken : public Token {};
    struct ProductToken : public Token {
        int value = -1;
    };
    struct SumToken: public Token{
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
    const int tokenMax = 10;//=counter base
    int vectorFlag;
    int matrixFlag;
    int resultFlag;
    //seed variables
    int streamIter = 0;
    int sMode = 0; // 0 = matrix, 1 = vector, 2 = none
    int stopReceiveDir=-1;
    bool columnFinished = false;
    std::vector<std::string> valueStream;
};

class Matrix2System : public AmoebotSystem
{
public:
    Matrix2System(int numParticles = 10, int countValue = 250);

    virtual bool hasTerminated() const;
};

#endif // Matrix_h
