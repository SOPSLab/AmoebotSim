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
#include <string.h>
#include "compact.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace Compact {
    CompactFlag::CompactFlag()
        : point(false),
          followIndicator(false),
          nextFollowDirOffset(-1)
    {}

    CompactFlag::CompactFlag(const CompactFlag& other)
        : Flag(other),
          state(other.state),
          point(other.point),
          followIndicator(other.followIndicator),
          contractDir(other.contractDir),
          nextFollowDirOffset(other.nextFollowDirOffset)
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
            system->insert(Particle(new Compact(State::Idle), randDir(), head, -1));
        }
        return system;
    }

    Movement Compact::execute() {
        if(state == State::Seed) { // State::Finished will eventually be included here as well
            followDir = -1;
            headMarkDir = -1;
            unsetFollowIndicator();

            return Movement(MovementType::Idle);
        }

        /* IDEA: let the seed become a normal particle at some point

        if(state == State::Seed && !hasNeighborInState(State::Idle)) {
            setState(State::Active);
            return Movement(MovementType::Idle);
        }*/

        // ORIENTATION: IN-PROGRESS
        if(state == State::Idle) {
            auto label = firstNeighborInState(State::Seed);
            
            if(label == -1) {
                label = firstNeighborInState(State::Active);
            }

            if(label != -1) {
                setState(State::Active);
                followDir = label;
                setFollowIndicatorLabel(followDir);
                headMarkDir = followDir;
            }

            return Movement(MovementType::Idle);
        }

        // MOVEMENT AFTER EXPANSION: IN-PROGRESS
        // complete a movement if the particle is expanded in the appropriate manner (handover or normal contraction)
        if(isExpanded() && !hasNeighborInState(State::Idle)) { // do not contract away from a particle that hasn't oriented yet
            Q_ASSERT(state == State::Leader || state == State::Follower); // the only particles that do expansion contraction should be Leaders and Followers

            setFollowIndicatorLabel(followDir);

            if(!tailReceivesFollowIndicator() && !hasNeighborInState(State::Leader)) { // particle does not have children in the spanning tree
                // TODO: reset next follow direction offsets?
                setState(State::Active);
                qDebug() << "doing a normal contraction, rand timestamp: " << randDir(); // DEBUG: This is definitely the case where the spanning trees pull apart
                return Movement(MovementType::Contract, tailContractionLabel());
            }
            else if(followIndicatorMatchState(State::Follower) || followIndicatorMatchState(State::Active)) { // I don't really like the Active piece here
                // reset next follow direction offsets?
                setState(State::Active);
                qDebug() << "tried to do a handover contract, rand timestamp: " << randDir();
                return Movement(MovementType::HandoverContract, tailContractionLabel());
            }
            else {
                qDebug() << "can't contract yet";
                return Movement(MovementType::Idle);
            }
        }

        if(state == State::Active) {
            if(hasNeighborInState(State::Idle)) {
                return Movement(MovementType::Idle);
            }
            else if(inFlags[followDir]->state == State::Leader) {
                setState(State::Follower);

                int expandDir = followDir;
                setContractDir(expandDir);
                followDir = (followDir + inFlags[followDir]->nextFollowDirOffset) % 6; // may need to double-check this works
                headMarkDir = followDir;
                setFollowIndicatorLabel(dirAfterExpansionTable[followDir][expandDir]);

                return Movement(MovementType::Expand, expandDir);
            }
            else if(inFlags[followDir]->state == State::Follower) {
                setState(State::Follower);

                int expandDir = followDir;
                setContractDir(expandDir);
                followDir = updatedFollowDir(); // revisit this function, but it does work
                headMarkDir = followDir;
                //setFollowIndicatorLabel(dirAfterExpansionTable[followDir][expandDir]);
                setFollowIndicatorLabel(dirToHeadLabelAfterExpansion(followDir, expandDir));

                return Movement(MovementType::Expand, expandDir);
            }
            else if(!isLocallyCompact() && receivesFollowIndicator() && !hasNeighborInState(State::Leader) && !hasNeighborInState(State::Follower)) {
                setState(State::Leader);

                int expandDir = firstEmptyNeighbor();
                setContractDir(expandDir);
                setNextFollowDirOffset(followDir, expandDir);
                followDir = (expandDir + 3) % 6; // direct itself at the space it was just in
                headMarkDir = followDir;
                setFollowIndicatorLabel(dirAfterExpansionTable[followDir][expandDir]);

                return Movement(MovementType::Expand, expandDir);
            }
            /*else if(!receivesFollowIndicator() && !hasNeighborInState(State::Leader) && !hasNeighborInState(State::Follower)) {
                // perform a leaf switch
                followDir = firstNeighborInState(State::Active);
                headMarkDir = followDir;
                setFollowIndicatorLabel(followDir);

                return Movement(MovementType::Idle);
            }*/
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

    bool Compact::receivesFollowIndicator() const {
        for(int i = 0; i < 10; i++) {
            if(inFlags[i] != nullptr && inFlags[i]->followIndicator) {
                return true;
            }
        }

        return false;
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
        // look for empty space starting at head direction and proceeding counter-clockwise, should only be done on particles ready to move
        Q_ASSERT(isContracted());
        Q_ASSERT(!isLocallyCompact());

        for(int label = followDir; label < 6; label++) {
            if(inFlags[label] == nullptr) {
                return label;
            }
        }
        for(int label = 0; label < followDir; label++) {
            if(inFlags[label] == nullptr) {
                return label;
            }
        }

        return -1;
    }

    void Compact::setNextFollowDirOffset(const int currentFollowDir, const int expandDir) {
        if(currentFollowDir == -1) {
            for(int label = 0; label < 10; ++label) {
                outFlags[label].nextFollowDirOffset = -1; // temporary; how to reset when dealing with offsets?
            }
        }
        else {
            for(int label = 0; label < 6; ++label) {
                if(inFlags[label] != nullptr && label != currentFollowDir) { // only set nextFollowDirOffset for potential children
                    outFlags[dirAfterExpansionTable[label][expandDir]].nextFollowDirOffset = (currentFollowDir - label + 9) % 6;
                }
            }            
        }
    }
}