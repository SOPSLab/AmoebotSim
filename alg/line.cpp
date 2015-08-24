#include <set>
#include <random>
#include <QTime>
#include "line.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace Line
{
LineFlag::LineFlag()
    : point(false),
      followIndicator(false)
{
}

LineFlag::LineFlag(const LineFlag& other)
    : Flag(other),
      state(other.state),
      point(other.point),
      followIndicator(other.followIndicator)
{
}

Line::Line(const State _state)
    : state(_state),
      followDir(-1)
{
    if (_state == State::Seed){
        outFlags[0].point = true;
        outFlags[3].point = true;
        headMarkDir = 0;
    }
    setState(_state);
}

Line::Line(const Line& other)
    : AlgorithmWithFlags(other),
      state(other.state),
      followDir(other.followDir)
{
}

Line::~Line()
{
}

std::shared_ptr<System> Line::instance(const unsigned int size, const double holeProb)
{
    std::shared_ptr<System> system = std::make_shared<System>();
    std::set<Node> occupied, candidates;

    // Create Seed Particle
    system->insertParticle(Particle(std::make_shared<Line>(State::Seed), randDir(), Node(0,0), -1));
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
        system->insertParticle(Particle(std::make_shared<Line>(State::Idle), randDir(), head, -1));
    }
    return system;
}

Movement Line::execute()
{
    if(isExpanded()){
        if(state == State::Follower) {
            setFollowIndicatorLabel(followDir);
        }

        if(hasNeighborInState(State::Idle) || (tailReceivesFollowIndicator() && (followIndicatorMatchState(State::Follower) || (followIndicatorMatchState(State::Leader) && state != State::Follower)))) {
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        }
        else {
            return Movement(MovementType::Contract, tailContractionLabel());
        }
    }
    else {
        if (state == State::Idle){
            if (hasNeighborInState(State::Finished) || hasNeighborInState(State::Seed)){
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
            }
        }

        else if (state == State::Follower && !hasNeighborInState(State::Idle)) {

            if (hasNeighborInState(State::Finished)){
                setState(State::Leader);
            }
            else {
                if(inFlags[followDir] == nullptr){
                    if (hasNeighborInState(State::Leader)){
                        followDir = firstNeighborInState(State::Leader);
                    }
                    else {
                        followDir = firstNeighborInState(State::Follower);
                    }
                }
                if(inFlags[followDir]->isExpanded() && !inFlags[followDir]->fromHead) {
                    int expansionDir = followDir;
                    setContractDir(expansionDir);
                    followDir = updatedFollowDir();
                    headMarkDir = followDir;
                    auto temp = dirToHeadLabelAfterExpansion(followDir, expansionDir);
                    Q_ASSERT(labelToDirAfterExpansion(temp, expansionDir) == followDir);
                    setFollowIndicatorLabel(temp);
                    return Movement(MovementType::Expand, expansionDir);
                }
            }
        }

        // Leader: Only changes to finished if pointed at, and sets point to continue in straight line
        else if (state == State::Leader && !hasNeighborInState(State::Idle)){
            headMarkDir = -1;
            int direction = isPointedAt();
            headMarkDir = direction;
            if(direction != -1){
                setState(State::Finished);
                outFlags[(direction+3)%6].point = true; // Set point
            }
            else {
                auto moveDir = getMoveDir();
                setContractDir(moveDir);
                headMarkDir = moveDir;
                return Movement(MovementType::Expand, moveDir);
            }
        }

        else if (state == State::Finished || state == State::Seed){
            return Movement(MovementType::Empty);
        }
        return Movement(MovementType::Idle);
    }
}

std::shared_ptr<Algorithm> Line::blank() const
{
    return std::make_shared<Line>(State::Idle);
}

std::shared_ptr<Algorithm> Line::clone()
{
    return std::make_shared<Line>(*this);
}

bool Line::isDeterministic() const
{
    return true;
}

int Line::isPointedAt(){
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr) {
            if(inFlags[label]->point){
                return label;
            }
        }
    }
    return -1;
}

void Line::setState(State _state)
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

bool Line::neighborInState(int direction, State _state){
    Q_ASSERT(0 <= direction && direction <= 9);
    return (inFlags[direction] != nullptr && inFlags[direction]->state == _state);
}

bool Line::hasNeighborInState(State _state)
{
    return (firstNeighborInState(_state) != -1);
}

int Line::firstNeighborInState(State _state)
{
    for(int label = 0; label < 10; label++) {
        if(neighborInState(label, _state)) {
            return label;
        }
    }
    return -1;
}

int Line::getMoveDir()
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

void Line::setContractDir(const int contractDir)
{
    for(int label = 0; label < 10; label++) {
        outFlags[label].contractDir = contractDir;
    }
}
int Line::updatedFollowDir() const
{
    int contractDir = inFlags[followDir]->contractDir;
    int offset = (followDir - inFlags[followDir]->dir + 9) % 6;
    int tempFollowDir = (contractDir + offset) % 6;
    Q_ASSERT(0 <= tempFollowDir && tempFollowDir <= 5);
    return tempFollowDir;
}

void Line::unsetFollowIndicator()
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = false;
    }
}

void Line::setFollowIndicatorLabel(const int label)
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = (i == label);
    }
}

bool Line::tailReceivesFollowIndicator() const
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

bool Line::followIndicatorMatchState(State _state) const
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
