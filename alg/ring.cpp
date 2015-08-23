#include <set>
#include <random>
#include <QTime>
#include "ring.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace Ring
{
RingFlag::RingFlag()
    : point(false),
      stopper(false),
      followIndicator(false)
{
}

RingFlag::RingFlag(const RingFlag& other)
    : Flag(other),  
      state(other.state),
      point(other.point),
      stopper(other.stopper),
      followIndicator(other.followIndicator),
      contractDir(other.contractDir)
{
}

Ring::Ring(const State _state)
    : state(_state),
      followDir(-1),
      wait(0)
{
    if (_state == State::Seed){
        outFlags[0].point = true;
        headMarkDir = 0;
        followDir = 0;
    }
    setState(_state);
}


Ring::Ring(const Ring& other)
    : AlgorithmWithFlags(other),
      state(other.state),
      followDir(other.followDir),
      wait(other.wait)
{
}

Ring::~Ring()
{
}

std::shared_ptr<System> Ring::instance(const unsigned int size, const double holeProb)
{
    std::shared_ptr<System> system = std::make_shared<System>();
    std::set<Node> occupied, candidates;

    // Create Seed Particle
    system->insert(Particle(std::make_shared<Ring>(State::Seed), randDir(), Node(0,0), -1));
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
        system->insert(Particle(std::make_shared<Ring>(State::Idle), randDir(), head, -1));
    }
    return system;
}

std::shared_ptr<Algorithm> Ring::blank()
{
    return std::make_shared<Ring>(State::Idle);
}

std::shared_ptr<Algorithm> Ring::clone()
{
    return std::make_shared<Ring>(*this);
}

bool Ring::isDeterministic() const
{
    return true;
}

Movement Ring::execute()
{
    if(isExpanded()){
        if(state == State::Follower) {
            setFollowIndicatorLabel(followDir);
        }
        // Sorry, long exception statement needed its own 3 lines
        if  (hasNeighborInState(State::Idle) || 
            state == State::Leader2 || state == State::Follower2 ||
            (tailReceivesFollowIndicator() && (followIndicatorMatchState(State::Follower) || (followIndicatorMatchState(State::Leader) && state != State::Follower)))) {
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        } 
        else {
            return Movement(MovementType::Contract, tailContractionLabel());
        }
    }
    else {
        if (state == State::Idle){
            if (hasNeighborInState(State::Set) || hasNeighborInState(State::Seed)){
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
            if (hasNeighborInState(State::Set)){
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

        else if (state == State::Leader && !hasNeighborInState(State::Idle)){
            headMarkDir = -1;
            int direction = isPointedAt();
            if(direction != -1){
                setState(State::Set); // Normal hexagon formation
                if(neighborInState(direction, State::Seed)){
                    direction = (direction+1)%6;
                }
                else if (neighborInState((direction+2)%6, State::Set) || neighborInState((direction+2)%6, State::Follower2)){
                    direction = (direction+3)%6;
                }
                else {
                    direction = (direction+2)%6;
                }
                setPoint(direction);
                headMarkDir = direction;
                followDir = direction;
            }
            else {
                auto moveDir = getMoveDir();
                setContractDir(moveDir);
                headMarkDir = moveDir;
                return Movement(MovementType::Expand, moveDir);
            }
        }

        else if ((state == State::Follower2 || state == State::Seed) && !hasNeighborInState(State::Idle)){
            if (hasNeighborInState(State::Leader2) && hasNeighborInState(State::Seed)){
                setStopper(); // Set Stopper indicator
            }
            if (inFlags[followDir] == nullptr && state == State::Seed){
                return Movement(MovementType::Idle);
            }
            else if (neighborInState(followDir, State::Finished)){
                setState(State::Finished);
            }
            else {
                if(inFlags[followDir]->isExpanded() && !inFlags[followDir]->fromHead) {
                    int expansionDir = followDir;
                    setContractDir(expansionDir);
                    followDir = updatedFollowDir();
                    headMarkDir = followDir;
                    auto temp = dirToHeadLabelAfterExpansion(followDir, expansionDir);
                    Q_ASSERT(labelToDirAfterExpansion(temp, expansionDir) == followDir);
                    setFollowIndicatorLabel(temp);
                    setPoint(expansionDir);
                    return Movement(MovementType::Expand, expansionDir);
                }
            }
        }

        else if (state == State::Leader2){
            if (nearStopper()){ // Checks for stopper particle (in mid section)
                setState(State::Finished);
            }
            else {
                auto moveDir = getMoveDir();
                setContractDir(moveDir);
                headMarkDir = moveDir;
                setPoint(moveDir);
                return Movement(MovementType::Expand, moveDir);
            }
        }

        else if (state == State::Set && !hasNeighborInState(State::Leader) && !hasNeighborInState(State::Follower)){
            auto label = isPointedAt();
            if  (neighborInState(label,State::Follower2) || neighborInState(label,State::Seed)){   
                if (wait < 5) {
                    wait++; // inclusion of a constant wait period is a rudimentary way to solve the issue of having some particles still being in hex formation
                }
                else {
                    if (inFlags[headMarkDir] != nullptr){
                        setState(State::Follower2);
                    }
                    else {
                        setState(State::Leader2);
                    }
                    setPoint(headMarkDir);
                }
            }
        }

        else if (state == State::Finished){
            return Movement(MovementType::Empty);
        }
        
    }
    return Movement(MovementType::Idle);
}

int Ring::isPointedAt(){
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr) {
            if(inFlags[label]->point){
                return label;
            }
        }
    }
    return -1;
}

void Ring::setPoint(int _label){
    for(int label = 0; label < 10; label++) {
        outFlags[label].point = false;
    }
    if (_label != -1) {
        outFlags[_label].point = true;
    }
}

bool Ring::nearStopper(){
    for (int label = 0; label < 10; label++){
        if (inFlags[label] != nullptr && inFlags[label]->stopper){
            return true;
        }
    }
    return false;
}
void Ring::setStopper(bool value){
    for (int label = 0; label < 10; ++label)
    {
        outFlags[label].stopper = value;
    }
}

void Ring::setState(State _state)
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
    else if(state == State::Set) {
        headMarkColor = 0x777777; tailMarkColor = 0x777777; // Gray
    } 
    else if(state == State::Leader2) {
        headMarkColor = 0xaa0000; tailMarkColor = 0xaa0000; // Dark Red
    } 
    else if(state == State::Follower2) {
        headMarkColor = 0x0000aa; tailMarkColor = 0x0000aa; // Dark Blue
    }
    else { // phase == Phase::Idle
        headMarkColor = -1; tailMarkColor = -1; // No color
    }
    for(int i = 0; i < 10; ++i) {
        outFlags[i].state = state;
    }
}

bool Ring::neighborInState(int direction, State _state){
    Q_ASSERT(0 <= direction && direction <= 9);
    return (inFlags[direction] != nullptr && inFlags[direction]->state == _state);
}

bool Ring::hasNeighborInState(State _state)
{
    return (firstNeighborInState(_state) != -1);
}

int Ring::firstNeighborInState(State _state)
{
    for(int label = 0; label < 10; label++) {
        if(neighborInState(label, _state)) {
            return label;
        }
    }
    return -1;
}

int Ring::getMoveDir()
{
    Q_ASSERT(isContracted());
    int objectDir = -1;
    if (state == State::Leader){
        objectDir = firstNeighborInState(State::Set);
        if (hasNeighborInState(State::Seed)){
            objectDir = firstNeighborInState(State::Seed);
        }
        if (hasNeighborInState(State::Follower2)){
            objectDir = firstNeighborInState(State::Follower2);
        }
        objectDir = (objectDir+5)%6;
        if (neighborInState(objectDir, State::Set) || neighborInState(objectDir, State::Follower2)){
            objectDir = (objectDir+5)%6;
        }
    }
    else {
        objectDir = (firstNeighborInState(State::Follower2)+1)%6;
        if (neighborInState(objectDir, State::Finished)){
            objectDir = (objectDir+1)%6;
        }
        if (neighborInState(objectDir, State::Follower2)){
            objectDir = (objectDir+1)%6;
        }
        if (neighborInState(objectDir, State::Follower2)){
            objectDir = (objectDir+1)%6;
        }
    }
    return labelToDir(objectDir);
}

void Ring::setContractDir(const int contractDir)
{
    for(int label = 0; label < 10; label++) {
        outFlags[label].contractDir = contractDir;
    }
}
int Ring::updatedFollowDir() const
{
    int contractDir = inFlags[followDir]->contractDir;
    int offset = (followDir - inFlags[followDir]->dir + 9) % 6;
    int tempFollowDir = (contractDir + offset) % 6;
    Q_ASSERT(0 <= tempFollowDir && tempFollowDir <= 5);
    return tempFollowDir;
}

void Ring::unsetFollowIndicator()
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = false;
    }
}

void Ring::setFollowIndicatorLabel(const int label)
{
    for(int i = 0; i < 10; i++) {
        outFlags[i].followIndicator = (i == label);
    }
}

bool Ring::tailReceivesFollowIndicator() const
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

bool Ring::followIndicatorMatchState(State _state) const
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
