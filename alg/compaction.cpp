#include <set>
#include <random>

#include "compaction.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace Compaction
{
CompactionFlag::CompactionFlag()
    : isParent(false),
      oldFollowDir(-1)
{
}

CompactionFlag::CompactionFlag(const CompactionFlag& other)
    : Flag(other),
      state(other.state),
      isParent(other.isParent),
      oldFollowDir(other.oldFollowDir)
{
}

Compaction::Compaction(const State _state)
    : state(_state),
      followDir(-1)
{
    setState(_state);
}

Compaction::Compaction(const Compaction& other)
    : AlgorithmWithFlags(other),
      state(other.state),
      followDir(other.followDir)
{
}

Compaction::~Compaction()
{
}

System* Compaction::instance(const unsigned int size, const double holeProb)
{
    System* system = new System();
    std::set<Node> occupied, candidates;

    // Create Seed Particle
    system->insert(Particle(new Compaction(State::Seed), randDir(), Node(0,0), -1));
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
        system->insert(Particle(new Compaction(State::Idle), randDir(), head, -1));
    }
    return system;
}

Movement Compaction::execute()
{
    if(state == State::Seed) {
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
            // leaf switch
            if(isLeaf() && countNeighbors() != 6 && !hasNeighborInState(State::Follower) && !hasNeighborInState(State::Leader)) {
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

Algorithm* Compaction::clone()
{
    return new Compaction(*this);
}

bool Compaction::isDeterministic() const
{
    return false; // uses randomization in leaf switch
}

void Compaction::setState(const State _state)
{
    state = _state;
    if(state == State::Seed) {
        headMarkColor = 0x00ff00; tailMarkColor = 0x00ff00; // Green
    } else if(state == State::Leader) {
        headMarkColor = 0xcc0000; tailMarkColor = 0xcc0000; // Red
    } else if(state == State::Follower) {
        headMarkColor = 0x3366ff; tailMarkColor = 0x3366ff; // Blue
    } else if(state == State::Active) {
        headMarkColor = 0x505050; tailMarkColor = 0xe0e0e0; // Grey
    } else { // phase == Phase::Idle
        headMarkColor = -1; tailMarkColor = -1; // No color
    }

    for(int i = 0; i < 10; i++) {
        outFlags[i].state = state;
    }
}

void Compaction::setOldFollowDir(const int dir)
{
    Q_ASSERT(0 <= dir && dir < 6);
    for(int label = 0; label < 10; label++) {
        outFlags[label].oldFollowDir = dir;
    }
}

void Compaction::setParentLabel(const int parentLabel)
{
    Q_ASSERT(0 <= parentLabel && parentLabel < 10);

    for(int label = 0; label < 10; label++) {
        outFlags[label].isParent = (label == parentLabel);
    }
}

void Compaction::unsetParentLabel() {
    for(int label = 0; label < 10; label++) {
        outFlags[label].isParent = false;
    }
}

bool Compaction::hasNeighborInState(const State _state) const
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr) {
            const CompactionFlag& flag = *inFlags[label];
            if(flag.state == _state) {
                return true;
            }
        }
    }
    return false;
}

int Compaction::neighborInStateDir(const State _state) const
{
    Q_ASSERT(isContracted());
    for(int dir = 0; dir < 6; dir++) {
        if(inFlags[dir] != nullptr) {
            const CompactionFlag& flag = *inFlags[dir];
            if(flag.state == _state) {
                return dir;
            }
        }
    }
    return -1;
}

int Compaction::emptyNeighborDir() const
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

    Q_ASSERT(false);
    return -1;
}

int Compaction::leafSwitchDir() const
{
    Q_ASSERT(isLeaf());
    Q_ASSERT(isContracted());

    int switchDir;

    while(true) {
        switchDir = randDir();
        if(inFlags[switchDir] != nullptr && (inFlags[switchDir]->state == State::Active || inFlags[switchDir]->state == State::Seed)) {
            break;
        }
    }

    return switchDir;
}

bool Compaction::isLocallyCompact() const
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
        int dir = (firstOccupiedDir + offset) % 6;
        if(inFlags[dir] != nullptr) {
            adjacentCount++;
        } else {
            break;
        }
    }

    return adjacentCount == count;
}

bool Compaction::isParent() const
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr && inFlags[label]->isParent) {
            return true;
        }
    }
    return false;
}

bool Compaction::isLeaf() const
{
    return !isParent();
}

bool Compaction::tailHasChild() const
{
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        auto label = *it;
        if(inFlags[label] != nullptr && inFlags[label]->isParent) {
            return true;
        }
    }
    return false;
}

bool Compaction::leaderAsChild() const
{
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        auto label = *it;
        if(inFlags[label] != nullptr && inFlags[label]->state == State::Leader && inFlags[label]->dir == inFlags[label]->oldFollowDir) {
            return true;
        }
    }
    return false;
}

int Compaction::countNeighbors() const
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
