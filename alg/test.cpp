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
    for(int label = 0; label < 6; label++) {
        if(hasNeighborAtLabel(label)) {
            if(neighborAtLabel<TestParticle>(label).state != State::Idle) {
                state = State::Follow;
            }
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

int TestParticle::headMarkDir() const
{
    return -1;
}

TestSystem::TestSystem()
{
    particles.push_back(new TestParticle(Node(0, 0), -1, randDir(), particleMap, State::Seed));
    particleMap[Node(0, 0)] = particles.back();

    for(int x = 1; x < 10; x++) {
        Node node(x, 0);
        particles.push_back(new TestParticle(node, -1, randDir(), particleMap, State::Idle));
        particleMap[node] = particles.back();
    }
}

}
