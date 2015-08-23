#include <set>
#include <random>

#include "holeelimstandard.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace HoleElimStandard
{
HoleElimStandardFlag::HoleElimStandardFlag()
    : isParent(false),
      dockingIndicator(false)
{
}

HoleElimStandardFlag::HoleElimStandardFlag(const HoleElimStandardFlag& other)
    : Flag(other),
      state(other.state),
      isParent(other.isParent),
      dockingIndicator(other.dockingIndicator)
{
}

HoleElimStandard::HoleElimStandard(const State _state)
    : state(_state),
      followDir(-1)
{
    setState(_state);
}

HoleElimStandard::HoleElimStandard(const HoleElimStandard& other)
    : AlgorithmWithFlags(other),
      state(other.state),
      followDir(other.followDir)
{
}

HoleElimStandard::~HoleElimStandard()
{
}

std::shared_ptr<System> HoleElimStandard::instance(const unsigned int size)
{
    std::shared_ptr<System> system = std::make_shared<System>();
    std::set<Node> occupied, seedComponent;
    std::deque<Node> queue;

    // define a square region of (2*size) particles, where every position has a 0.5 chance of being occupied
    int regionSize = sqrt(2.0 * size);
    for(int x = 0; x < regionSize; ++x) {
        for(int y = 0; y < regionSize; ++y) {
            if(randBool()) {
                occupied.insert(Node(x,y));
            }
        }
    }

    // randomly choose one of the occupied positions to be the seed
    int seedIndex = randInt(0, occupied.size());
    int i = 0;
    Node seed;
    for(auto it = occupied.begin(); it != occupied.end(); ++it) {
        if(i == seedIndex) {
            seed = *it;
            occupied.erase(it);
            break;
        }
        ++i;
    }

    // perform a flooding search beginning at the seed to discover the seed's connected component
    queue.push_back(seed);
    while(!queue.empty()) {
        Node n = queue.front();
        queue.pop_front();
        for(int dir = 0; dir < 6; ++dir) {
            Node neighbor = n.nodeInDir(dir);
            auto nodeIt = occupied.find(neighbor);
            if(nodeIt != occupied.end()) {
                queue.push_back(neighbor);
                seedComponent.insert(neighbor);
                occupied.erase(nodeIt);
            }
        }
    }

    // insert the seed and all particles from its component into the system
    system->insert(Particle(std::make_shared<HoleElimStandard>(State::Seed), randDir(), seed, -1));
    while(!seedComponent.empty()) {
        auto node = *seedComponent.begin();
        seedComponent.erase(seedComponent.begin());
        system->insert(Particle(std::make_shared<HoleElimStandard>(State::Idle), randDir(), node, -1));
    }

    return system;
}

Movement HoleElimStandard::execute()
{
    if(state == State::Seed || state == State::Finished) {
        return Movement(MovementType::Empty);
    }

    if(isContracted()) {
        if(state == State::Idle) {
            // if adjacent to the seed or a finished particle, become a walking particle
            followDir = neighborInStateDir(State::Seed);
            if(followDir == -1) {
                followDir = neighborInStateDir(State::Finished);
            }
            if(followDir != -1) {
                setState(State::Walking);
                headMarkDir = followDir;
                setParentLabel(followDir);
                return Movement(MovementType::Idle);
            }
            // otherwise, if adjacent to a walking particle or follower, become a follower
            followDir = neighborInStateDir(State::Walking);
            if(followDir == -1) {
                followDir = neighborInStateDir(State::Follower);
            }
            if(followDir != -1) {
                setState(State::Follower);
                headMarkDir = followDir;
                setParentLabel(followDir);
                return Movement(MovementType::Idle);
            } else { // has all idle neighbors, and can wait for a change
                return Movement(MovementType::Empty);
            }
        } else if(state == State::Walking) {
            if(hasNeighborInState(State::Seed)) { // finish and set a docking label on the axis
                setState(State::Finished);
                followDir = neighborInStateDir(State::Seed);
                headMarkDir = followDir;
                setParentLabel(followDir);
                setDockingLabel((followDir + 3) % 6);
                return Movement(MovementType::Idle);
            } else { // adjacent to some finished particle(s)
                // become finished if possible
                //if(dockingDir() != -1 && randInt(0,9) == 0) { // used to walk over axis positions, which generally creates a more convex structure
                if(dockingDir() != -1) {
                    setState(State::Finished);
                    followDir = dockingDir();
                    headMarkDir = followDir;
                    setParentLabel(followDir);
                    setDockingLabel((followDir + 3) % 6);
                    return Movement(MovementType::Idle);
                } else if(adjFinishDir() != -1) {
                    setState(State::Finished);
                    followDir = adjFinishDir();
                    headMarkDir = followDir;
                    setParentLabel(followDir);
                    return Movement(MovementType::Idle);
                }

                // otherwise, expand around the border, if possible
                if(borderFinishedDir() != -1) {
                    int borderDir = borderFinishedDir();
                    int expandDir = (borderDir + 1) % 6;
                    followDir = (borderDir + 5) % 6;
                    headMarkDir = followDir;
                    setParentLabel(dirToHeadLabelAfterExpansion(followDir, expandDir));
                    return Movement(MovementType::Expand, expandDir);
                }
            }
        } else if(state == State::Follower) {
            if(hasNeighborInState(State::Finished)) {
                setState(State::Walking);
                followDir = neighborInStateDir(State::Finished);
                headMarkDir = followDir;
                setParentLabel(followDir);
                return Movement(MovementType::Idle);
            } else if(inFlags[followDir]->isExpanded() && !inFlags[followDir]->fromHead) {
                int expandDir = followDir;
                followDir = (followDir + inFlags[followDir]->tailDir - inFlags[followDir]->dir + 6) % 6;
                headMarkDir = followDir;
                setParentLabel(dirToHeadLabelAfterExpansion(followDir, expandDir));
                return Movement(MovementType::Expand, expandDir);
            }
        }
    } else if(isExpanded() && !hasNeighborInState(State::Idle)) { // do not disconnect the system by contracting away from an unoriented particle
        setParentLabel(followDir);
        return tailHasChild() ? Movement(MovementType::HandoverContract, tailContractionLabel()) : Movement(MovementType::Contract, tailContractionLabel());
    }

    return Movement(MovementType::Idle);
}

std::shared_ptr<Algorithm> HoleElimStandard::blank()
{
    return std::make_shared<HoleElimStandard>(State::Idle);
}

std::shared_ptr<Algorithm> HoleElimStandard::clone()
{
    return std::make_shared<HoleElimStandard>(*this);
}

bool HoleElimStandard::isDeterministic() const
{
    return true;
}

void HoleElimStandard::setState(const State _state)
{
    state = _state;
    if(state == State::Seed) {
        headMarkColor = 0x00ff00; tailMarkColor = 0x00ff00; // Green
    } else if(state == State::Idle) {
        headMarkColor = -1; tailMarkColor = -1; // No color
    } else if(state == State::Walking) {
        headMarkColor = 0xffb000; tailMarkColor = 0xffb000; // Yellow
    } else if(state == State::Follower) {
        headMarkColor = 0x0066ff; tailMarkColor = 0x0066ff; // Blue
    } else { // phase == Phase::Finished
        headMarkColor = 0x000000; tailMarkColor = 0x000000; // Black
    }

    for(int i = 0; i < 10; i++) {
        outFlags[i].state = state;
    }
}

void HoleElimStandard::setParentLabel(const int parentLabel)
{
    Q_ASSERT(0 <= parentLabel && parentLabel < 10);

    for(int label = 0; label < 10; label++) {
        outFlags[label].isParent = (label == parentLabel);
    }
}

void HoleElimStandard::setDockingLabel(const int dockingLabel)
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= dockingLabel && dockingLabel < 6);

    outFlags[dockingLabel].dockingIndicator = true;
}

bool HoleElimStandard::hasNeighborInState(const State _state) const
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr && inFlags[label]->state == _state) {
            return true;
        }
    }
    return false;
}

int HoleElimStandard::neighborInStateDir(const State _state) const
{
    Q_ASSERT(isContracted());
    for(int dir = 0; dir < 6; dir++) {
        if(inFlags[dir] != nullptr && inFlags[dir]->state == _state) {
            return dir;
        }
    }
    return -1;
}

int HoleElimStandard::dockingDir() const
{
    Q_ASSERT(isContracted()); // only contracted particles should dock/finish

    for(int dir = 0; dir < 6; dir++) {
        if(inFlags[dir] != nullptr && inFlags[dir]->dockingIndicator) {
            return dir;
        }
    }
    return -1; // does not receive a docking indicator
}

int HoleElimStandard::adjFinishDir() const
{
    Q_ASSERT(isContracted());

    for(int dir = 0; dir < 6; dir++) {
        if(inFlags[dir] != nullptr && inFlags[dir]->state == State::Finished) {
            int left = (dir + 1) % 6, right = (dir + 5) % 6;
            if((inFlags[left] != nullptr && inFlags[left]->state == State::Finished) && (inFlags[right] != nullptr && inFlags[right]->state == State::Finished)) {
                return dir;
            }
        }
    }
    return -1;
}

int HoleElimStandard::borderFinishedDir() const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(followDir != -1);
    Q_ASSERT(inFlags[followDir] != nullptr);

    for(int offset = 0; offset < 6; offset++) {
        int dir = (followDir + offset) % 6;
        if(inFlags[dir] != nullptr && inFlags[dir]->state == State::Finished && inFlags[(dir + 1) % 6] == nullptr) {
            return dir;
        }
    }
    return -1;
}

bool HoleElimStandard::tailHasChild() const
{
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        auto label = *it;
        if(inFlags[label] != nullptr && inFlags[label]->isParent) {
            return true;
        }
    }
    return false;
}

}
