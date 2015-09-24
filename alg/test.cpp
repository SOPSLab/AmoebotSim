#include "test.h"

namespace Test
{

TestParticle::TestParticle(const Node head,
                           const int globalTailDir,
                           const int orientation,
                           std::map<Node, AmoebotParticle *> &particleMap,
                           State state)
    : AmoebotParticle(head, globalTailDir, orientation, particleMap),
      state(state),
      dir(-1)
{

}

void TestParticle::activate()
{
    if(state == State::Seed) {
        return;
    } else if(state == State::Idle) {
        int label = labelOfFirstNeighborInState({State::Seed, State::Follow});
        if(label != -1) {
            dir = label;
            state = State::Follow;
        }
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

int TestParticle::headMarkGlobalDir() const
{
    if(dir == -1) {
        return -1;
    } else {
        return localToGlobalDir(dir);
    }
}

TestParticle& TestParticle::neighborAtLabel(int label)
{
    return dynamic_cast<TestParticle&>(AmoebotParticle::neighborAtLabel(label));
}

int TestParticle::labelOfFirstNeighborInState(std::initializer_list<State> states)
{
    auto propertyCheck = [&](const TestParticle& p) {
        for(auto state : states) {
            if(p.state == state) {
                return true;
            }
        }
        return false;
    };

    return labelOfFirstNeighborWithProperty<TestParticle>(propertyCheck);
}

bool TestParticle::hasNeighborInState(std::initializer_list<State> states)
{
    return labelOfFirstNeighborInState(states) != -1;
}

TestSystem::TestSystem()
{
    insert(new TestParticle(Node(0, 0), -1, randDir(), particleMap, State::Seed));

    for(int x = 1; x < 10; x++) {
        insert(new TestParticle(Node(x, 0), -1, randDir(), particleMap, State::Idle));
    }
}

}
