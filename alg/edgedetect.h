#ifndef EDGEDETECT_H
#define EDGEDETECT_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"
#include <iostream>
#include <fstream>
#include <string>
class EdgeDetectParticle : public AmoebotParticle
{
    friend class EdgeDetectSystem;

public:
    enum class State
    {
        Seed,
        Idle,
        Follow,
        Lead,//3
        Finish,
        Active,
        Matrix,//6
        Vector,
        Result,//8
        Prestop

    };
    enum class TokenType
    {
        MatrixToken,
        VectorToken,
        EndOfColumnToken,
        EndOfVectorToken
    };

    EdgeDetectParticle(const Node head,
                    const int globalTailDir,
                    const int orientation,
                    AmoebotSystem& system,
                    State state);

    virtual void activate();

    virtual int headMarkColor() const;
    virtual int headMarkDir() const;
    virtual int tailMarkColor() const;

    virtual QString inspectionText() const;

    EdgeDetectParticle& neighborAtLabel(int label) const;

    int labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel = 0) const;
    bool hasNeighborInState(std::initializer_list<State> states) const;

    int constructionReceiveDir() const;
    bool stopFlagReceived() const;
    bool canFinish() const;
    int tryVectorStop() const;
    int tryMatrixStop() const;
    int tryResultStop() const;
    bool completeNeighborhood() const;
    bool missingGridVal() const;
    bool noTokensAtLabel(int label);
    int getTwoHopVal(int sourceNeighbor,int offset) const;
    void updateConstructionDir();

    void updateMoveDir();

    bool hasTailFollower() const;
    void setCounterGoal(int goal);
    bool shouldStop() const;
    bool tunnelCheck() const;

protected:
  /*  struct VectorToken : public Token {
        int value = -1;
    };
    struct MatrixToken : public Token {
        int value = -1;
    };
    struct EndOfColumnToken : public Token {};
    struct EndOfVectorToken : public Token {};
    */
    struct StreamToken : public Token {
        int value = -1;
        TokenType type;

    };
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
    const int MaxValue = 300;
    bool sentProduct = false;
    bool lastCol = false;
    const int tokenMax =4 ;//=counter base
    int vectorFlag;
    int matrixFlag;
    int resultFlag;
    int stopFlag;
    //seed variables
    int streamIter = 0;
    int sMode = 0; // 0 = matrix, 1 = vector, 2 = none
    int resultRound=0;
    int gridvals[8];
    int stopReceiveDir=-1;
    bool columnFinished = false;
    int numcountsgenerated = 0;
    int vectorLeftover = -1;
    int pixelVal;
    double gradientMagnitude;
    double gradientDir;
    std::vector<std::string> valueStream;
    std::shared_ptr<Token> firsttoken;
    std::shared_ptr<Token> secondtoken;
};


class EdgeDetectSystem : public AmoebotSystem
{
public:
    EdgeDetectSystem(int numParticles = 10, int countValue = 250);

    virtual bool hasTerminated() const;
};

#endif // EDGEDETECT_H
