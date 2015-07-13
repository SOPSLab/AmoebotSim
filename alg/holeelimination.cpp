#include <set>
#include <random>

#include "holeelimination.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace HoleElimination
{
HoleEliminationFlag::HoleEliminationFlag()
    : isParent(false),
      oldFollowDir(-1),
      dockingIndicator(false)
{
}

HoleEliminationFlag::HoleEliminationFlag(const HoleEliminationFlag& other)
    : Flag(other),
      state(other.state),
      isParent(other.isParent),
      oldFollowDir(other.oldFollowDir),
      dockingIndicator(other.dockingIndicator)
{
}

HoleElimination::HoleElimination(const State _state)
    : state(_state),
      followDir(-1)
{
    setState(_state);
}

HoleElimination::HoleElimination(const HoleElimination& other)
    : AlgorithmWithFlags(other),
      state(other.state),
      followDir(other.followDir)
{
}

HoleElimination::~HoleElimination()
{
}

std::shared_ptr<System> HoleElimination::instance(const unsigned int size, const double holeProb)
{
    std::shared_ptr<System> system = std::make_shared<System>();
    std::set<Node> occupied, candidates;

    // Create Seed Particle
    system->insert(Particle(std::make_shared<HoleElimination>(State::Seed), randDir(), Node(0,0), -1));
    occupied.insert(Node(0,0));

    for(int dir = 0; dir<6; dir++){
        candidates.insert(Node(0,0).nodeInDir(dir));
    }

    while(occupied.size() < size && !candidates.empty()){
        auto index = randInt(0, candidates.size());
        auto it = candidates.begin();
        while (index != 0){
            ++it;
            index--;
        }

        Node head = *it;
        candidates.erase(it);
        occupied.insert(head);

        if(randBool(holeProb)){
            continue;
        }

        for(int dir = 0; dir < 6; dir++){
            auto neighbor = head.nodeInDir(dir);
            if(occupied.find(neighbor) == occupied.end() && candidates.find(neighbor) == candidates.end()){
                candidates.insert(neighbor);
            }
        }
        // Insert new idle particle
        system->insert(Particle(std::make_shared<HoleElimination>(State::Idle), randDir(), head, -1));
    }
    return system;
}

Movement HoleElimination::execute()
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

            // become a follower of leader
            if(inFlags[followDir]->state == State::Leader && !inFlags[followDir]->fromHead) {
                setState(State::Follower);
                int expandDir = followDir;
                followDir = (followDir + inFlags[followDir]->oldFollowDir - inFlags[followDir]->dir + 9) % 6;
                headMarkDir = followDir;
                setParentLabel(dirToHeadLabelAfterExpansion(followDir, expandDir));
                return Movement(MovementType::Expand, expandDir);
            }
            // become a follower of follower
            if(inFlags[followDir]->state == State::Follower && !inFlags[followDir]->fromHead) {
                setState(State::Follower);
                int expandDir = followDir;
                followDir = (followDir + inFlags[followDir]->tailDir - inFlags[followDir]->dir + 6) % 6;
                headMarkDir = followDir;
                setParentLabel(dirToHeadLabelAfterExpansion(followDir, expandDir));
                return Movement(MovementType::Expand, expandDir);
            }

            // become finished
            if(hasNeighborInState(State::Seed)) {
                setState(State::Finished);
                followDir = neighborInStateDir(State::Seed);
                headMarkDir = followDir;
                setParentLabel(followDir);
                setDockingLabel((followDir + 3) % 6);
                return Movement(MovementType::Idle);
            } else if(dockingDir() != -1) {
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
            // become a roaming leaf
            if(isLeaf() && hasNeighborInState(State::Finished) && borderFinishedDir() != -1 && !hasNeighborInState(State::Follower) && !hasNeighborInState(State::Leader)) {
                setState(State::Follower);
                auto borderDir = borderFinishedDir();
                int expandDir;
                if(inFlags[(borderDir + 1) % 6] == nullptr) {
                    expandDir = (borderDir + 1) % 6;
                    followDir = (borderDir + 5) % 6;
                    headMarkDir = followDir;
                } else { // inFlags[(borderDir + 5) % 6] == nullptr
                    expandDir = (borderDir + 5) % 6;
                    followDir = (borderDir + 1) % 6;
                    headMarkDir = followDir;
                }
                setParentLabel(dirToHeadLabelAfterExpansion(followDir, expandDir));
                return Movement(MovementType::Expand, expandDir);
            }
            // leaf switch
            if(isLeaf() && !hasNeighborInState(State::Follower) && !hasNeighborInState(State::Leader)) {
                followDir = leafSwitchDir();
                headMarkDir = followDir;
                setParentLabel(followDir);
                return Movement(MovementType::Idle);
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
        }
    }

    return Movement(MovementType::Idle);
}

std::shared_ptr<Algorithm> HoleElimination::clone()
{
    return std::make_shared<HoleElimination>(*this);
}

bool HoleElimination::isDeterministic() const
{
    return false; // uses randomization in leaf switch
}

void HoleElimination::setState(const State _state)
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
    } else { // phase == Phase::Finished
        headMarkColor = 0x000000; tailMarkColor = 0x000000; // Black
    }

    for(int i = 0; i < 10; i++) {
        outFlags[i].state = state;
    }
}

void HoleElimination::setOldFollowDir(const int dir)
{
    Q_ASSERT(0 <= dir && dir < 6);
    for(int label = 0; label < 10; label++) {
        outFlags[label].oldFollowDir = dir;
    }
}

void HoleElimination::setParentLabel(const int parentLabel)
{
    Q_ASSERT(0 <= parentLabel && parentLabel < 10);

    for(int label = 0; label < 10; label++) {
        outFlags[label].isParent = (label == parentLabel);
    }
}

void HoleElimination::unsetParentLabel()
{
    for(int label = 0; label < 10; label++) {
        outFlags[label].isParent = false;
    }
}

void HoleElimination::setDockingLabel(const int dockingLabel)
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= dockingLabel && dockingLabel < 6);

    outFlags[dockingLabel].dockingIndicator = true;
}

void HoleElimination::unsetDockingLabel()
{
    for(int label = 0; label < 10; label++) {
        outFlags[label].dockingIndicator = false;
    }
}

bool HoleElimination::hasNeighborInState(const State _state) const
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr && inFlags[label]->state == _state) {
            return true;
        }
    }
    return false;
}

int HoleElimination::neighborInStateDir(const State _state) const
{
    Q_ASSERT(isContracted());
    for(int dir = 0; dir < 6; dir++) {
        if(inFlags[dir] != nullptr && inFlags[dir]->state == _state) {
            return dir;
        }
    }
    return -1;
}

int HoleElimination::emptyNeighborDir() const
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

int HoleElimination::leafSwitchDir() const
{
    Q_ASSERT(isLeaf());
    Q_ASSERT(isContracted());

    int switchDir;

    while(true) {
        switchDir = randDir();
        if(inFlags[switchDir] != nullptr && (inFlags[switchDir]->state == State::Active || inFlags[switchDir]->state == State::Seed || inFlags[switchDir]->state == State::Finished)) {
            break;
        }
    }

    return switchDir;
}

int HoleElimination::dockingDir() const
{
    Q_ASSERT(isContracted()); // only contracted particles should dock/finish

    for(int dir = 0; dir < 6; dir++) {
        if(inFlags[dir] != nullptr && inFlags[dir]->dockingIndicator) {
            return dir;
        }
    }
    return -1; // does not receive a docking indicator
}

int HoleElimination::adjFinishDir() const
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

int HoleElimination::borderFinishedDir() const
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

bool HoleElimination::isLocallyCompact() const
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

bool HoleElimination::isParent() const
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr && inFlags[label]->isParent) {
            return true;
        }
    }
    return false;
}

bool HoleElimination::isLeaf() const
{
    return !isParent();
}

bool HoleElimination::tailHasChild() const
{
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        auto label = *it;
        if(inFlags[label] != nullptr && inFlags[label]->isParent) {
            return true;
        }
    }
    return false;
}

bool HoleElimination::leaderAsChild() const
{
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        auto label = *it;
        if(inFlags[label] != nullptr && inFlags[label]->state == State::Leader && inFlags[label]->dir == inFlags[label]->oldFollowDir) {
            return true;
        }
    }
    return false;
}

int HoleElimination::countNeighbors() const
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

