#include <set>

#include <QtGlobal>

#include "hexagon.h"

HexagonParticle::HexagonParticle(const Node head,
                                 const int globalTailDir,
                                 const int orientation,
                                 std::map<Node, AmoebotParticle *> &particleMap,
                                 State state)
    : AmoebotParticle(head, globalTailDir, orientation, particleMap),
      state(state),
      constructionDir(-1),
      moveDir(-1),
      followDir(-1)
{
    if(state == State::Seed) {
        constructionDir = 0;
    }
}

void HexagonParticle::activate()
{
    if(isExpanded()) {
        if(state == State::Follow) {
            if(!hasNeighborInState({State::Idle}) && !hasTailFollower()) {
                contractTail();
            }
            return;
        } else if(state == State::Lead) {
            if(!hasNeighborInState({State::Idle}) && !hasTailFollower()) {
                contractTail();
                updateMoveDir();
            }
            return;
        } else {
            Q_ASSERT(false);
        }
    } else {
        if(state == State::Seed) {
            return;
        } else if(state == State::Idle) {
            if(hasNeighborInState({State::Seed, State::Finish})) {
                state = State::Lead;
                updateMoveDir();
                return;
            } else if(hasNeighborInState({State::Lead, State::Follow})) {
                state = State::Follow;
                followDir = labelOfFirstNeighborInState({State::Lead, State::Follow});
                return;
            }
        } else if(state == State::Follow) {
            if(hasNeighborInState({State::Seed, State::Finish})) {
                state = State::Lead;
                updateMoveDir();
                return;
            } else if(hasTailAtLabel(followDir)) {
                auto neighbor = neighborAtLabel(followDir);
                int neighborContractionDir = neighborDirToDir(neighbor, (neighbor.tailDir() + 3) % 6);
                expand(followDir);
                followDir = neighborContractionDir;
                return;
            }
        } else if(state == State::Lead) {
            if(canFinish()) {
                state = State::Finish;
                updateConstructionDir();
                return;
            } else {
                updateMoveDir();
                if(!hasNeighborAtLabel(moveDir) || hasTailAtLabel(moveDir)) {
                    expand(moveDir);
                }
                return;
            }
        }
    }
}

int HexagonParticle::headMarkColor() const
{
    switch(state) {
    case State::Seed:   return 0x00ff00;
    case State::Idle:   return -1;
    case State::Follow: return 0x0000ff;
    case State::Lead:   return 0xff0000;
    case State::Finish: return 0x000000;
    }
}

int HexagonParticle::headMarkGlobalDir() const
{
    if(state == State::Lead) {
        return localToGlobalDir(moveDir);
    } else if(state == State::Seed || state == State::Finish) {
        return localToGlobalDir(constructionDir);
    } else if(state == State::Follow) {
        return localToGlobalDir(followDir);
    }
    return -1;
}

int HexagonParticle::tailMarkColor() const
{
    return headMarkColor();
}

HexagonParticle& HexagonParticle::neighborAtLabel(int label) const
{
    return dynamic_cast<HexagonParticle&>(AmoebotParticle::neighborAtLabel(label));
}

int HexagonParticle::labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel) const
{
    auto propertyCheck = [&](const HexagonParticle& p) {
        for(auto state : states) {
            if(p.state == state) {
                return true;
            }
        }
        return false;
    };

    return labelOfFirstNeighborWithProperty<HexagonParticle>(propertyCheck, startLabel);
}

bool HexagonParticle::hasNeighborInState(std::initializer_list<State> states) const
{
    return labelOfFirstNeighborInState(states) != -1;
}

int HexagonParticle::constructionReceiveDir() const
{
    auto propertyCheck = [&](const HexagonParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Finish) &&
                pointsAtMe(p, p.constructionDir);
    };

    return labelOfFirstNeighborWithProperty<HexagonParticle>(propertyCheck);
}

bool HexagonParticle::canFinish() const
{
    return constructionReceiveDir() != -1;
}

void HexagonParticle::updateConstructionDir()
{
    constructionDir = constructionReceiveDir();
    if(neighborAtLabel(constructionDir).state == State::Seed) {
        constructionDir = (constructionDir + 1) % 6;
    } else {
        constructionDir = (constructionDir + 2) % 6;
    }

    if(hasNeighborAtLabel(constructionDir) && neighborAtLabel(constructionDir).state == State::Finish) {
        constructionDir = (constructionDir + 1) % 6;
    }
}

void HexagonParticle::updateMoveDir()
{
    moveDir = labelOfFirstNeighborInState({State::Seed, State::Finish});
    while(hasNeighborAtLabel(moveDir) && (neighborAtLabel(moveDir).state == State::Seed || neighborAtLabel(moveDir).state == State::Finish)) {
        moveDir = (moveDir + 5) % 6;
    }
}

bool HexagonParticle::hasTailFollower() const
{
    auto propertyCheck = [&](const HexagonParticle& p) {
        return  p.state == State::Follow &&
                pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
    };
    return labelOfFirstNeighborWithProperty<HexagonParticle>(propertyCheck) != -1;
}

HexagonSystem::HexagonSystem(int numParticles, float holeProb)
{
    insert(new HexagonParticle(Node(0, 0), -1, randDir(), particleMap, HexagonParticle::State::Seed));

    std::set<Node> occupied;
    occupied.insert(Node(0, 0));

    std::set<Node> candidates;
    for(int i = 0; i < 6; i++) {
        candidates.insert(Node(0, 0).nodeInDir(i));
    }

    // add inactive particles
    int numNonStaticParticles = 0;
    while(numNonStaticParticles < numParticles && !candidates.empty()) {
        // pick random candidate
        int randIndex = randInt(0, candidates.size());
        Node randomCandidate;
        for(auto it = candidates.begin(); it != candidates.end(); ++it) {
            if(randIndex == 0) {
                randomCandidate = *it;
                candidates.erase(it);
                break;
            } else {
                randIndex--;
            }
        }

        occupied.insert(randomCandidate);

        if(randBool(1.0f - holeProb)) {
            // only add particle if not a hole
            insert(new HexagonParticle(randomCandidate, -1, randDir(), particleMap, HexagonParticle::State::Idle));
            numNonStaticParticles++;

            // add new candidates
            for(int i = 0; i < 6; i++) {
                auto neighbor = randomCandidate.nodeInDir(i);
                if(occupied.find(neighbor) == occupied.end()) {
                    candidates.insert(neighbor);
                }
            }
        }
    }
}

bool HexagonSystem::hasTerminated() const
{
    if(!isConnected()) {
        return true;
    }

    for(auto p : particles) {
        auto hp = dynamic_cast<HexagonParticle*>(p);
        if(hp->state != HexagonParticle::State::Seed && hp->state != HexagonParticle::State::Finish) {
            return false;
        }
    }

    return true;
}