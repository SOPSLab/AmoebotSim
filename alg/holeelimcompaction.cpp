#include <set>
#include <random>

#include "holeelimcompaction.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace HoleElimCompaction
{
HoleElimCompactionFlag::HoleElimCompactionFlag()
    : isParent(false),
      oldFollowDir(-1),
      dockingIndicator(false)
{
}

HoleElimCompactionFlag::HoleElimCompactionFlag(const HoleElimCompactionFlag& other)
    : Flag(other),
      state(other.state),
      isParent(other.isParent),
      oldFollowDir(other.oldFollowDir),
      dockingIndicator(other.dockingIndicator)
{
}

HoleElimCompaction::HoleElimCompaction(const State _state)
    : state(_state),
      followDir(-1)
{
    setState(_state);
}

HoleElimCompaction::HoleElimCompaction(const HoleElimCompaction& other)
    : AlgorithmWithFlags(other),
      state(other.state),
      followDir(other.followDir)
{
}

HoleElimCompaction::~HoleElimCompaction()
{
}

std::shared_ptr<System> HoleElimCompaction::instance(const unsigned int size)
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
    system->insertParticle(Particle(std::make_shared<HoleElimCompaction>(State::Seed), randDir(), seed, -1));
    while(!seedComponent.empty()) {
        auto node = *seedComponent.begin();
        seedComponent.erase(seedComponent.begin());
        system->insertParticle(Particle(std::make_shared<HoleElimCompaction>(State::Idle), randDir(), node, -1));
    }

    return system;
}

Movement HoleElimCompaction::execute()
{
    if(state == State::Seed || state == State::Finished) {
        return Movement(MovementType::Empty);
    }

    if(isContracted()) {
        if(state == State::Idle) {
            followDir = neighborInStateDir(State::Seed);
            if(followDir == -1) {
                followDir = neighborInStateDir(State::Active);
            }
            if(followDir == -1) {
                return Movement(MovementType::Empty);
            } else {
                setState(State::Active);
                headMarkDir = followDir;
                setParentLabel(followDir);
                return Movement(MovementType::Idle);
            }
        } else if(state == State::Active) {
            // do nothing until your neighbors are oriented
            if(hasNeighborInState(State::Idle)) {
                return Movement(MovementType::Idle);
            }

            // become a follower of a leader
            if(inFlags[followDir]->state == State::Leader && !inFlags[followDir]->fromHead) {
                setState(State::Follower);
                int expandDir = followDir;
                followDir = (followDir + inFlags[followDir]->oldFollowDir - inFlags[followDir]->dir + 9) % 6;
                headMarkDir = followDir;
                setParentLabel(dirToHeadLabelAfterExpansion(followDir, expandDir));
                return Movement(MovementType::Expand, expandDir);
            }
            // become a follower of a follower or walking particle
            if((inFlags[followDir]->state == State::Follower || inFlags[followDir]->state == State::Walking) && !inFlags[followDir]->fromHead) {
                setState(State::Follower);
                int expandDir = followDir;
                followDir = (followDir + inFlags[followDir]->tailDir - inFlags[followDir]->dir + 6) % 6;
                headMarkDir = followDir;
                setParentLabel(dirToHeadLabelAfterExpansion(followDir, expandDir));
                return Movement(MovementType::Expand, expandDir);
            }

            // become walking
            if(hasNeighborInState(State::Seed) || hasNeighborInState(State::Finished)) {
                setState(State::Walking);
                followDir = neighborInStateDir(State::Seed);
                if(followDir == -1) {
                    followDir = neighborInStateDir(State::Finished);
                }
                headMarkDir = followDir;
                setParentLabel(followDir);
                return Movement(MovementType::Idle);
            }

            // become a leader
            if(isParent() && !isLocallyCompact() && !hasNeighborInState(State::Follower)) {
                setState(State::Leader);
                setOldFollowDir(followDir);
                auto dir = emptyNeighborDir();
                int tailDirAfterExpansion = (dir + 3) % 6;
                followDir = tailDirAfterExpansion;
                headMarkDir = tailDirAfterExpansion;
                unsetParentLabel();
                return Movement(MovementType::Expand, dir);
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
        }
    } else { // isExpanded
        if(state == State::Leader) {
            setState(State::Active);
            setParentLabel(followDir);
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        } else if(state == State::Follower) {
            if(tailHasChild()) {
                setState(State::Active);
                setParentLabel(followDir);
                return Movement(MovementType::HandoverContract, tailContractionLabel());
            } else if(leaderAsChild()) {
                return Movement(MovementType::Idle);
            } else {
                setState(State::Active);
                setParentLabel(followDir);
                return Movement(MovementType::Contract, tailContractionLabel());
            }
        } else if(state == State::Walking && !leaderAsChild()) {
            setParentLabel(followDir);
            return tailHasChild() ? Movement(MovementType::HandoverContract, tailContractionLabel()) : Movement(MovementType::Contract, tailContractionLabel());
        }
    }

    return Movement(MovementType::Idle);
}

std::shared_ptr<Algorithm> HoleElimCompaction::blank() const
{
    return std::make_shared<HoleElimCompaction>(State::Idle);
}

std::shared_ptr<Algorithm> HoleElimCompaction::clone()
{
    return std::make_shared<HoleElimCompaction>(*this);
}

bool HoleElimCompaction::isDeterministic() const
{
    return false; // uses randomization in leaf switch
}

bool HoleElimCompaction::isStatic() const
{
    return false;
}

void HoleElimCompaction::setState(const State _state)
{
    state = _state;
    if(state == State::Seed) {
        headMarkColor = 0x00ff00; tailMarkColor = 0x00ff00; // Green
    } else if(state == State::Idle) {
        headMarkColor = -1; tailMarkColor = -1; // No color
    } else if(state == State::Active) {
        headMarkColor = 0x505050; tailMarkColor = 0xe0e0e0; // Grey
    } else if(state == State::Leader) {
        headMarkColor = 0xcc0000; tailMarkColor = 0xcc0000; // Red
    } else if(state == State::Follower) {
        headMarkColor = 0x0066ff; tailMarkColor = 0x0066ff; // Blue
    } else if(state == State::Walking) {
        headMarkColor = 0xffb000; tailMarkColor = 0xffb000; // Yellow
    } else { // phase == Phase::Finished
        headMarkColor = 0x000000; tailMarkColor = 0x000000; // Black
    }

    for(int i = 0; i < 10; i++) {
        outFlags[i].state = state;
    }
}

void HoleElimCompaction::setOldFollowDir(const int dir)
{
    Q_ASSERT(0 <= dir && dir < 6);
    for(int label = 0; label < 10; label++) {
        outFlags[label].oldFollowDir = dir;
    }
}

void HoleElimCompaction::setParentLabel(const int parentLabel)
{
    Q_ASSERT(0 <= parentLabel && parentLabel < 10);

    for(int label = 0; label < 10; label++) {
        outFlags[label].isParent = (label == parentLabel);
    }
}

void HoleElimCompaction::unsetParentLabel()
{
    for(int label = 0; label < 10; label++) {
        outFlags[label].isParent = false;
    }
}

void HoleElimCompaction::setDockingLabel(const int dockingLabel)
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= dockingLabel && dockingLabel < 6);

    outFlags[dockingLabel].dockingIndicator = true;
}

bool HoleElimCompaction::hasNeighborInState(const State _state) const
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr && inFlags[label]->state == _state) {
            return true;
        }
    }
    return false;
}

int HoleElimCompaction::neighborInStateDir(const State _state) const
{
    Q_ASSERT(isContracted());
    for(int dir = 0; dir < 6; dir++) {
        if(inFlags[dir] != nullptr && inFlags[dir]->state == _state) {
            return dir;
        }
    }
    return -1;
}

int HoleElimCompaction::emptyNeighborDir() const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(followDir != -1); // must have a followDir
    Q_ASSERT(inFlags[followDir] != nullptr); // can't be oriented to an empty space

    for(int offset = 1; offset <= 3; offset++) {
        int left = (followDir + offset) % 6;
        int right = (followDir - offset + 6) % 6;
        if(inFlags[right] == nullptr) {
            return right;
        } else if(inFlags[left] == nullptr) {
            return left;
        }
    }
    return -1;
}

int HoleElimCompaction::dockingDir() const
{
    Q_ASSERT(isContracted()); // only contracted particles should dock/finish

    for(int dir = 0; dir < 6; dir++) {
        if(inFlags[dir] != nullptr && inFlags[dir]->dockingIndicator) {
            return dir;
        }
    }
    return -1; // does not receive a docking indicator
}

int HoleElimCompaction::adjFinishDir() const
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

int HoleElimCompaction::borderFinishedDir() const
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
    for(int offset = 0; offset < 6; offset++) {
        int dir = (followDir + offset) % 6;
        if(inFlags[dir] != nullptr && inFlags[dir]->state == State::Finished && inFlags[(dir + 5) % 6] == nullptr) {
            return dir;
        }
    }
    return -1;
}

bool HoleElimCompaction::isLocallyCompact() const
{
    Q_ASSERT(isContracted());

    int count = countNeighbors();

    if(0 == count || count == 6) {
        return true;
    } else if(count == 5) {
        return false;
    }

    int firstOccupiedDir = -1;
    for(int dir = 0; dir < 6; dir++) {
        if(inFlags[dir] != nullptr && inFlags[(dir + 5) % 6] == nullptr) {
            firstOccupiedDir = dir;
            break;
        }
    }

    int adjacentCount = 0;
    for(int offset = 0; offset < 6; offset++) {
        if(inFlags[(firstOccupiedDir + offset) % 6] != nullptr) {
            adjacentCount++;
        } else {
            break;
        }
    }

    return adjacentCount == count;
}

bool HoleElimCompaction::isParent() const
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr && inFlags[label]->isParent) {
            return true;
        }
    }
    return false;
}

bool HoleElimCompaction::isLeaf() const
{
    return !isParent();
}

bool HoleElimCompaction::tailHasChild() const
{
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        auto label = *it;
        if(inFlags[label] != nullptr && inFlags[label]->isParent) {
            return true;
        }
    }
    return false;
}

bool HoleElimCompaction::leaderAsChild() const
{
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        auto label = *it;
        if(inFlags[label] != nullptr && inFlags[label]->state == State::Leader && inFlags[label]->dir == inFlags[label]->oldFollowDir) {
            return true;
        }
    }
    return false;
}

int HoleElimCompaction::countNeighbors() const
{
    int count = 0;
    int max = (isContracted()) ? 6 : 10;

    for(int label = 0; label < max; label++) { // FIX: doesn't this construction double count the side neighbors for expanded particles?
        if(inFlags[label] != nullptr) {
            count++;
        }
    }
    return count;
}

}


