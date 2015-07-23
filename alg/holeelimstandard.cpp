#include <set>
#include <random>

#include "holeelimcoating.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace HoleElimCoating
{
HoleElimCoatingFlag::HoleElimCoatingFlag()
    : isParent(false),
      dockingIndicator(false)
{
}

HoleElimCoatingFlag::HoleElimCoatingFlag(const HoleElimCoatingFlag& other)
    : Flag(other),
      state(other.state),
      isParent(other.isParent),
      dockingIndicator(other.dockingIndicator)
{
}

HoleElimCoating::HoleElimCoating(const State _state)
    : state(_state),
      followDir(-1)
{
    setState(_state);
}

HoleElimCoating::HoleElimCoating(const HoleElimCoating& other)
    : AlgorithmWithFlags(other),
      state(other.state),
      followDir(other.followDir)
{
}

HoleElimCoating::~HoleElimCoating()
{
}

System* HoleElimCoating::instance(const unsigned int size, const double holeProb)
{
    System* system = new System();
    std::set<Node> occupied, candidates;

    // Create Seed Particle
    system->insert(Particle(new HoleElimCoating(State::Seed), randDir(), Node(0,0), -1));
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
        system->insert(Particle(new HoleElimCoating(State::Idle), randDir(), head, -1));
    }

    return system;
}

Movement HoleElimCoating::execute()
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

Algorithm* HoleElimCoating::clone()
{
    return new HoleElimCoating(*this);
}

bool HoleElimCoating::isDeterministic() const
{
    return true;
}

void HoleElimCoating::setState(const State _state)
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

void HoleElimCoating::setParentLabel(const int parentLabel)
{
    Q_ASSERT(0 <= parentLabel && parentLabel < 10);

    for(int label = 0; label < 10; label++) {
        outFlags[label].isParent = (label == parentLabel);
    }
}

void HoleElimCoating::setDockingLabel(const int dockingLabel)
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= dockingLabel && dockingLabel < 6);

    outFlags[dockingLabel].dockingIndicator = true;
}

bool HoleElimCoating::hasNeighborInState(const State _state) const
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr && inFlags[label]->state == _state) {
            return true;
        }
    }
    return false;
}

int HoleElimCoating::neighborInStateDir(const State _state) const
{
    Q_ASSERT(isContracted());
    for(int dir = 0; dir < 6; dir++) {
        if(inFlags[dir] != nullptr && inFlags[dir]->state == _state) {
            return dir;
        }
    }
    return -1;
}

int HoleElimCoating::dockingDir() const
{
    Q_ASSERT(isContracted()); // only contracted particles should dock/finish

    for(int dir = 0; dir < 6; dir++) {
        if(inFlags[dir] != nullptr && inFlags[dir]->dockingIndicator) {
            return dir;
        }
    }
    return -1; // does not receive a docking indicator
}

int HoleElimCoating::adjFinishDir() const
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

int HoleElimCoating::borderFinishedDir() const
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

bool HoleElimCoating::tailHasChild() const
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
