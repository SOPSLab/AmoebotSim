/* FILENAME:    compact.cpp
 * AUTHOR:      Josh Daymude, borrowing heavily from the structure and algorithms written by Brian Parker
 * DESCRIPTION: Defines the algorithm for compacting a system of particles. A compact system is defined as
 *              a convex shape with no holes.
*/

 // NOTE: 0 - 5 increases counter-clockwise

#include <set>
#include <random>
#include <QTime>
#include <QDebug>
#include "compact.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace Compact {
    CompactFlag::CompactFlag()
        : point(false),
          followIndicator(false),
          nextFollowDir(-1)
    {}

    CompactFlag::CompactFlag(const CompactFlag& other)
        : Flag(other),
          state(other.state),
          point(other.point),
          followIndicator(other.followIndicator),
          contractDir(other.contractDir),
          nextFollowDir(other.nextFollowDir)
    {}

    Compact::Compact(const State _state)
        : state(_state),
          followDir(-1)
    {
        if (_state == State::Seed){
            outFlags[0].point = true;
            headMarkDir = 0;
        }
        setState(_state);
    }

    Compact::Compact(const Compact& other)
        : AlgorithmWithFlags(other),
          state(other.state),
          followDir(other.followDir)
    {}

    Compact::~Compact()
    {}

    System* Compact::instance(const unsigned int size, const double holeProb) {
        System* system = new System();
        std::set<Node> occupied, candidates;

        // Create Seed Particle
        system->insert(Particle(new Compact(State::Seed), randDir(), Node(0,0), -1));
        occupied.insert(Node(0,0));

        for(int dir = 0; dir<6;dir++){
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
            system->insert(Particle(new Compact(State::Idle), randDir(), head, -1));
        }
        return system;
    }

    Movement Compact::execute() {
        if(state == State::Finished || state == State::Seed) {
            return Movement(MovementType::Empty);
        }

        // complete a movement if the particle is expanded in the appropriate manner (handover or normal contraction)
        if(isExpanded()) {
            Q_ASSERT(state == State::Leader || state == State::Follower); // the only particles that do expansion contraction should be Leaders and Followers

            // want some way of recalculating compactness after a contraction
            /*if(isLocallyCompact()) {
                setState(State::Compact);
            }
            else {
                setState(State::NotCompact);
            }*/

            // wipe any possible nextFollowDir outflags
            // setNextFollowDir(-1, -1);

            if(tailReceivesFollowIndicator()) {
                if(followIndicatorMatchState(State::Follower)) {
                    setNextFollowDir(-1);
                    return Movement(MovementType::HandoverContract, tailContractionLabel());
                }
                else if(followIndicatorMatchState(State::Active)) {
                    return Movement(MovementType::Idle); // TODO: potential bottleneck, but it needs to wait until the child actually decides to follow in order to contract
                }
            }
            else {
                setNextFollowDir(-1);
                return Movement(MovementType::Contract, tailContractionLabel());
            }
        }

        if(state == State::Idle) {
            auto label = firstNeighborInState(State::Seed);
            if(label == -1) {
                label = std::max(std::max(firstNeighborInState(State::Active), firstNeighborInState(State::Compact)), firstNeighborInState(State::NotCompact));
            }

            if(label != -1) {
                setState(State::Active);
                followDir = labelToDir(label);
                setFollowIndicatorLabel(followDir);
                headMarkDir = followDir;

                return Movement(MovementType::Idle); // not sure we need a return here, could just keep going through the rest?
            }
        }

        if(state == State::Active || state == State::Compact || state == State::NotCompact) {
            // assumption: by the time a particle is Active, its inFlags[followDir] is never nullptr, we maintain the spanning tree
            if(inFlags[followDir]->isExpanded()) {
                // if we could assume that the only expanded particles are Leaders or Followers, we could make this part cleaner
                if(inFlags[followDir]->state == State::Leader) {
                    setState(State::Follower);

                    int expandDir = followDir;
                    setContractDir(expandDir);
                    qDebug() << "Point 0: followDir = " << followDir;
                    followDir = inFlags[followDir]->nextFollowDir; // orients follower to where the leader was pointing before expansion
                    qDebug() << "Point 1: followDir = " << followDir;
                    headMarkDir = followDir;
                    setFollowIndicatorLabel(dirToHeadLabelAfterExpansion(followDir, expandDir)); // not 100% sure what this does

                    return Movement(MovementType::Expand, expandDir);
                }
                else if(inFlags[followDir]->state == State::Follower) {
                    setState(State::Follower);

                    int expandDir = followDir;
                    setContractDir(expandDir);
                    followDir = updatedFollowDir(); // orients follower toward the contracted position of the particle in is following
                    headMarkDir = followDir;
                    qDebug() << "Point 2: followDir = " << followDir;
                    setFollowIndicatorLabel(dirToHeadLabelAfterExpansion(followDir, expandDir)); // not 100% sure what this does

                    return Movement(MovementType::Expand, expandDir);
                }
            }
        }
        
        if(!isLocallyCompact() && !hasNeighborInState(State::Idle) && !hasNeighborExpanded()) {
            setState(State::Leader);

            // choose a child if necessary and give it nextFollowDir
            /* int childDir = getChildDir();
            if(childDir != -1) { // otherwise, it doesn't need any followers because they don't exist
                setNextFollowDir(followDir, childDir);
                qDebug() << "nextFollowDir set: childDir = " << childDir << "\nfollowDir = " << followDir << "\noutFlags[" << childDir << "]->nextFollowDir = " << outFlags[childDir].nextFollowDir; 
            } */

            setNextFollowDir(followDir);
            int expandDir = getMoveDir();
            setContractDir(expandDir);
            followDir = (expandDir + 4) % 6; // +4 is the same as -2 mod 6
            headMarkDir = followDir;
            qDebug() << "Point 3: followDir = " << followDir;
            setFollowIndicatorLabel(dirToHeadLabelAfterExpansion(followDir, expandDir)); // pretty sure this is correct

            return Movement(MovementType::Expand, expandDir);
        }

        return Movement(MovementType::Idle);
    }

    Algorithm* Compact::clone() {
        return new Compact(*this);
    }

    bool Compact::isDeterministic() const {
        return true;
    }

    int Compact::isPointedAt() {
        for(int label = 0; label < 10; label++) {
            if(inFlags[label] != nullptr) {
                if(inFlags[label]->point) {
                    return label;
                }
            }
        }

        return -1;
    }
      
    void Compact::setState(State _state) {
        state = _state;
        if(state == State::Seed) {
            headMarkColor = 0x00ff00; tailMarkColor = 0x00ff00; // Green
        }
        else if(state == State::Finished) {
            headMarkColor = 0x000000; tailMarkColor = 0x000000; // Black
        }
        else if(state == State::Compact) {
            headMarkColor = 0xffcc00; tailMarkColor = 0xffcc00; // Gold
        }
        else if(state == State::NotCompact) {
            headMarkColor = 0xff6600; tailMarkColor = 0xff6600; // Orange
        }
        else if(state == State::Leader) {
            headMarkColor = 0xcc0000; tailMarkColor = 0xcc0000; // Red
        }
        else if(state == State::Follower) {
            headMarkColor = 0x3366FF; tailMarkColor = 0x3366FF; // Blue
        }
        else if(state == State::Active) {
            headMarkColor = 0x505050; tailMarkColor = 0xe0e0e0; // Grey
        }
        else { // phase == Phase::Idle
            headMarkColor = -1; tailMarkColor = -1; // No color
        }

        for(int i = 0; i < 10; i++) {
            outFlags[i].state = state;
        }
    }

    bool Compact::neighborInState(int direction, State _state) {
        Q_ASSERT(0 <= direction && direction <= 9);
        return (inFlags[direction] != nullptr && inFlags[direction]->state == _state);
    }

    bool Compact::hasNeighborInState(State _state) {
        return (firstNeighborInState(_state) != -1);
    }

    int Compact::firstNeighborInState(State _state) {
        for(int label = 0; label < 10; label++) {
            if(neighborInState(label, _state)) {
                return label;
            }
        }

        return -1;
    }

    int Compact::getMoveDir() {
        Q_ASSERT(isContracted());
        int objectDir = firstEmptyNeighbor();
        Q_ASSERT(objectDir != -1); // otherwise, this particle is completely surrounded, and thus is compact and shouldn't be moving

        return labelToDir(objectDir);
    }

    void Compact::setContractDir(const int contractDir) {
        for(int label = 0; label < 10; label++) {
            outFlags[label].contractDir = contractDir;
        }
    }

    int Compact::updatedFollowDir() const {
        int contractDir = inFlags[followDir]->contractDir;
        int offset = (followDir - inFlags[followDir]->dir + 9) % 6; // this works in hexagon.cpp, but it may be able to be done better?

        return (contractDir + offset) % 6;
    }

    void Compact::unsetFollowIndicator() {
        for(int i = 0; i < 10; i++) {
            outFlags[i].followIndicator = false;
        }
    }

    void Compact::setFollowIndicatorLabel(const int label) {
        for(int i = 0; i < 10; i++) {
            outFlags[i].followIndicator = (i == label);
        }
    }

    bool Compact::tailReceivesFollowIndicator() const {
        for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
            auto label = *it;

            if(inFlags[label] != nullptr) {
                if(inFlags[label]->followIndicator) {
                    return true;
                }
            }
        }

        return false;
    }

    bool Compact::followIndicatorMatchState(State _state) const {
        for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
            auto label = *it;

            if(inFlags[label] != nullptr) {
                if(inFlags[label]->followIndicator && inFlags[label]->state == _state) {
                    return true;
                }
            }
        }

        return false;
    }

    // my added functions:

    int Compact::neighborCount() {
        int count = 0;
        
        for(int label = 0; label < 10; ++label) {
            if(inFlags[label] != nullptr) {
                count++;
                /*if(inFlags[label]->isExpanded())
                    count--;*/
            }
        }

        return count;
    }

    // for now, assume this is only called on non-extended particles (maximum of 6 neighbors)
    // returns the largest number of adjacent neighbors
    int Compact::adjacentNeighborCount() {
        int adjCount = 0;
        int numNeighbors = neighborCount();

        switch(numNeighbors) {
            case 0: 
                qDebug() << "WARNING: adjacentNeighborCount called on a disconnected particle!";
                break;
            case 1: case 5: case 6: // if a particle has 1, 5, or 6 neighbors they must be adjacent
                adjCount = numNeighbors;
                break;
            case 2:
                adjCount = 1;
                for(int label = 0; label < 6; ++label) {
                    if(inFlags[label] != nullptr && inFlags[(label + 1) % 6] != nullptr) {
                        adjCount = 2;
                        break;
                    }
                }
                break;
            case 3:
                adjCount = 1;
                for(int label = 0; label < 6; ++label) {
                    if(inFlags[label] != nullptr && inFlags[(label + 1) % 6] != nullptr) {
                        adjCount = 2;
                    }
                    if(inFlags[label] != nullptr && inFlags[(label + 1) % 6] != nullptr && inFlags[(label + 2) % 6]) {
                        adjCount = 3;
                        break;
                    }
                }
                break;
            case 4:
                adjCount = 1;
                for(int label = 0; label < 6; ++label) {
                    if(inFlags[label] == nullptr && inFlags[(label + 1) % 6] == nullptr) {
                        adjCount = 4;
                        break;
                    }
                }
                break;
            default:
                qDebug() << "WARNING: adjacentNeighborCount called on particle with > 6 neighbors.";
        }

        return adjCount;
    }

    bool Compact::isLocallyCompact() {
        bool flag = false;
        int numNeighbors = neighborCount();
        int adjNeighbors = adjacentNeighborCount();

        if(numNeighbors == 5) {
            flag = false;
        }
        else if(numNeighbors == adjNeighbors) {
            flag = true;
        }

        return flag;
    }

    int Compact::firstEmptyNeighbor() {
        // look for empty space starting at head direction and proceeding clockwise, should only be done on particles ready to move
        Q_ASSERT(isContracted()); // can comment this out as long as getMoveDir() is the only caller

        for(int label = headMarkDir; label < 6; label++) {
            if(inFlags[label] == nullptr) {
                return label;
            }
        }
        for(int label = 0; label < headMarkDir; label++) {
            if(inFlags[label] == nullptr) {
                return label;
            }
        }

        return -1;
    }

    bool Compact::hasNeighborExpanded() {
        for(int label = 0; label < 10; label++) {
            if(inFlags[label] != nullptr && inFlags[label]->isExpanded()) {
                return true;
            }
        }

        return false;
    }

    int Compact::getChildDir() {
        Q_ASSERT(isContracted()); // should call this only before a movement

        int backLabel = (followDir + 3) % 6;
        for(int i = 0; i < 3; i++) {
            int left = (backLabel + i) % 6;
            int right = (backLabel - i) % 6;

            if(inFlags[left] != nullptr && inFlags[left]->followIndicator) {
                return left;
            }
            else if(inFlags[right] != nullptr && inFlags[right]->followIndicator) {
                return right;
            }
        }

        return -1;
    }

    void Compact::setNextFollowDir(const int nextFollowDir) {
        for (int label = 0; label < 10; ++label) {
            outFlags[label].nextFollowDir = nextFollowDir;
        }
    }

    /* void Compact::setNextFollowDir(const int nextFollowDir, const int childDir) {
        for(int label = 0; label < 10; label++) {
            if(label == childDir) {
                outFlags[label].nextFollowDir = nextFollowDir;
            }
            else {
                outFlags[label].nextFollowDir = -1;
            }
        }
    } */
}