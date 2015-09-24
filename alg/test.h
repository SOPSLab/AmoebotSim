#ifndef TEST_H
#define TEST_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

namespace Test
{

enum class State
{
    Seed,
    Idle,
    Follow,
    Lead,
    Finish
};

class TestParticle : public AmoebotParticle
{
    friend class TestSystem;

public:
    TestParticle(const Node head,
                 const int globalTailDir,
                 const int orientation,
                 std::map<Node, AmoebotParticle*>& particleMap,
                 State state);

    virtual void activate();

    virtual int headMarkColor() const;
    virtual int headMarkDir() const;

    virtual TestParticle& neighborAtLabel(int label);

    int labelOfFirstNeighborInState(std::initializer_list<State> states);
    bool hasNeighborInState(std::initializer_list<State> states);

private:
    State state;
};

class TestSystem : public AmoebotSystem
{
public:
    TestSystem();
};

}




#endif // TEST_H
