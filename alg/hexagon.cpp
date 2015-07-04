#include <set>
#include <random>
#include <QTime>
#include "hexagon.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace Hexagon
{
HexFlag::HexFlag()
    : point(false),
      followIndicator(false)
{
}

HexFlag::HexFlag(const HexFlag& other)
    : Flag(other),
      state(other.state),
      point(other.point),
      followIndicator(other.followIndicator)
{
}

Hexagon::Hexagon(const State _state)
    : state(_state),
      followDir(-1)
{
    if (_state == State::Seed){
        outFlags[0].point = true;
        headMarkDir = 0;
    }
    setState(_state);
}

Hexagon::Hexagon(const Hexagon& other)
    : AlgorithmWithFlags(other),
      state(other.state),
      followDir(other.followDir)
{
}

Hexagon::~Hexagon()
{
}

System* Hexagon::instance(const unsigned int size, const double holeProb)
{
    System* system = new System();
    std::set<Node> occupied, candidates;

    // Create Seed Particle
    system->insert(Particle(std::make_shared<Hexagon>(State::Seed), randDir(), Node(0,0), -1));
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
        system->insert(Particle(std::make_shared<Hexagon>(State::Idle), randDir(), head, -1));
    }
    return system;
}

Movement Hexagon::execute()
{
    if(isExpanded()){
        if(state == State::Follower) {
            setFollowIndicatorLabel(followDir);
        }

        if(hasNeighborInState(State::Idle) || (tailReceivesFollowIndicator() && (followIndicatorMatchState(State::Follower) || (followIndicatorMatchState(State::Leader) && state != State::Follower)))) {
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        } else {
            return Movement(MovementType::Contract, tailContractionLabel());
        }
    } else {
        if(state == State::Idle){
            if(hasNeighborInState(State::Finished) || hasNeighborInState(State::Seed)) {
                setState(State::Leader);
                return Movement(MovementType::Idle);
            }
            auto label = std::max(firstNeighborInState(State::Follower), firstNeighborInState(State::Leader));
            if(label != -1) {
                setState(State::Follower);
                followDir = labelToDir(label);
                setFollowIndicatorLabel(followDir);
                headMarkDir = followDir;
                return Movement(MovementType::Idle);
            } else {
                return Movement(MovementType::Empty);
            }
        } else if(state == State::Follower && !hasNeighborInState(State::Idle)) {
            if(hasNeighborInState(State::Finished)){
                setState(State::Leader);
                return Movement(MovementType::Idle);
            } else {
                if(inFlags[followDir]->isExpanded() && !inFlags[followDir]->fromHead) {
                    int expansionDir = followDir;
                    setContractDir(expansionDir);
                    followDir = updatedFollowDir();
                    headMarkDir = followDir;
                    auto temp = dirToHeadLabelAfterExpansion(followDir, expansionDir);
                    Q_ASSERT(labelToDirAfterExpansion(temp, expansionDir) == followDir);
                    setFollowIndicatorLabel(temp);
                    return Movement(MovementType::Expand, expansionDir);
                } else {
                    return Movement(MovementType::Empty);
                }
            }
        } else if(state == State::Leader && !hasNeighborInState(State::Idle)){
            headMarkDir = -1;
            int direction = isPointedAt();
            headMarkDir = direction;
            if(direction != -1) { // Change to finished state and use surroundings to find the next point direction
                setState(State::Finished);
                if(neighborInState(direction, State::Seed)){
                    outFlags[(direction+1)%6].point = true;
                } else if (neighborInState((direction+2)%6, State::Finished)){
                    outFlags[(direction+3)%6].point = true;
                } else {
                    outFlags[(direction+2)%6].point = true;
                }
                return Movement(MovementType::Idle);
            } else {
                auto moveDir = getMoveDir();
                setContractDir(moveDir);
                headMarkDir = moveDir;
                return Movement(MovementType::Expand, moveDir);
            }
        } else {
            return Movement(MovementType::Empty);
        }
    }
}

std::shared_ptr<Algorithm> Hexagon::clone()
{
    return std::make_shared<Hexagon>(*this);
}

bool Hexagon::isDeterministic() const
{
    return true;
}

int Hexagon::isPointedAt(){
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr) {
            if(inFlags[label]->point){
                return label;
            }
        }
    }
    return -1;
}

void Hexagon::setState(State _state)
{
    state = _state;
    if (state == State::Seed){
        headMarkColor = 0x00ff00; tailMarkColor = 0x00ff00; // Green
    }
    else if(state == State::Finished) {
        headMarkColor = 0x000000; tailMarkColor = 0x000000; // Black
    }
    else if(state == State::Leader) {
        headMarkColor = 0xff0000; tailMarkColor = 0xff0000; // Red
    }
    else if(state == State::Follower) {
        headMarkColor = 0x0000ff; tailMarkColor = 0x0000ff; // Blue
    }
    else { // phase == Phase::Idle
        headMarkColor = -1; tailMarkColor = -1; // No color
    }

    for(int i = 0; i < 10; i++) {
        outFlags[i].state = state;
    }
}

bool Hexagon::neighborInState(int direction, State _state){
    Q_ASSERT(0 <= direction && direction <= 9);
    return (inFlags[direction] != nullptr && inFlags[direction]->state == _state);
}

bool Hexagon::hasNeighborInState(State _state)
{
    return (firstNeighborInState(_state) != -1);
}

int Hexagon::firstNeighborInState(State _state)
{
    for(int label = 0; label < 10; label++) {
        if(neighborInState(label, _state)) {
            return label;
        }
    }
    return -1;
}

int Hexagon::getMoveDir()
{
    Q_ASSERT(isContracted());
    int objectDir = firstNeighborInState(State::Finished);
    if (hasNeighborInState(State::Seed)){
        objectDir = firstNeighborInState(State::Seed);
    }
    objectDir = (objectDir+5)%6;
    if (neighborInState(objectDir, State::Finished)){
        objectDir = (objectDir+5)%6;
    }
    return labelToDir(objectDir);
}

void Hexagon::setContractDir(const int contractDir)
{
    for(int label = 0; label < 10; label++) {
        outFlags[label].contractDir = contractDir;
    }
}
int Hexagon::updatedFollowDir() const
{
    int contractDir = inFlags[followDir]->contractDir;
    int offset = (followDir - inFlags[followDir]->dir + 9) % 6;
    int tempFollowDir = (contractDir + offset) % 6;
    Q_ASSERT(0 <= tempFollowDir && tempFollowDir <= 5);
    return tempFollowDir;
}

void Hexagon::unsetFollowIndicator()
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = false;
    }
}

void Hexagon::setFollowIndicatorLabel(const int label)
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = (i == label);
    }
}

bool Hexagon::tailReceivesFollowIndicator() const
{
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

bool Hexagon::followIndicatorMatchState(State _state) const
{
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

}
