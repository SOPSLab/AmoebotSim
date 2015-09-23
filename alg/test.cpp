#include "test.h"

namespace Test
{

TestParticle::TestParticle(const Node head,
                           const int globalTailDir,
                           const int orientation,
                           std::map<Node, AmoebotParticle *> &particleMap,
                           State state)
    : AmoebotParticle(head, globalTailDir, orientation, particleMap),
      state(state)
{

}

void TestParticle::activate()
{
    if(state == State::Seed) {
        return;
    }

    if(hasNeighborInState({State::Seed, State::Follow})) {
        state = State::Follow;
    }
}

int TestParticle::headMarkColor() const
{
    switch(state) {
    case State::Seed:
        return 0x00ff00;
        break;
    case State::Idle:
        return -1;
        break;
    case State::Follow:
        return 0x0000ff;
        break;
    case State::Lead:
        return 0xff0000;
        break;
    case State::Finish:
        return 0x000000;
        break;
    }
}

int TestParticle::headMarkDir() const
{
    return -1;
}

bool TestParticle::hasNeighborInState(std::initializer_list<State> states)
{
    for(int label = 0; label < 6; label++) {
        if(hasNeighborAtLabel(label)) {
            for(auto state : states) {
                if(neighborAtLabel<TestParticle>(label).state == state) {
                    return true;
                }
            }
        }
    }
    return false;
}

TestSystem::TestSystem()
{
    insert(new TestParticle(Node(0, 0), -1, randDir(), particleMap, State::Seed));

    for(int x = 1; x < 10; x++) {
        insert(new TestParticle(Node(x, 0), -1, randDir(), particleMap, State::Idle));
    }
}

}
