#ifndef ADDER_H
#define ADDER_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

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

public:
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

    AdderParticle& neighborAtLabel(int label) const;

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
    static const int numBits = 1;
   static const int numChannels = numBits;
    int bits[numBits];
    bool inC[numChannels];
    bool outC[numChannels];
    int prevLabel = -1;
    int nextLabel = -1;
    int index;
    int seedSend = 0;
private:
    int getOpenOut();
    void addInternal();
    void clearInternal();
    bool bitsOpen();

};

class AdderSystem : public AmoebotSystem
{
public:
    AdderSystem(int numParticles = 200, float holeProb = 0.2);

    virtual bool hasTerminated() const;
};

#endif // ADDER_h
