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
      followIndicator(false),
      stopper(false)
{
}

RingFlag::RingFlag(const RingFlag& other)
    : Flag(other),  
      state(other.state),
      point(other.point),
      followIndicator(other.followIndicator),
      stopper(other.stopper)
{
}

Ring::Ring(const State _state)
    : state(_state),
      followDir(-1),
      head(false)
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
      followDir(other.followDir)
{
}

Ring::~Ring()
{
}

System* Ring::instance(const int size, const double holeProb)
{
    System* system = new System();
    std::set<Node> occupied, candidates;

    // Create Seed Particle
    system->insert(Particle(new Ring(State::Seed), randDir(), Node(0,0), -1));
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
        system->insert(Particle(new Ring(State::Idle), randDir(), head, -1));
    }
    return system;
}

Algorithm* Ring::clone()
{
    return new Ring(*this);
}

bool Ring::isDeterministic() const
{
    return true;
}

Movement Ring::execute()
{
    if(isExpanded()/* && state != State::Finished*/){
        if(state == State::Follower) {
            setFollowIndicatorLabel(followDir);
        }
        // Sorry, long exception statement needed its own 3 lines
        if  (hasNeighborInState(State::Idle) || 
            state == State::Leader2 || state == State::Follower2 ||
            (tailReceivesFollowIndicator() && (followIndicatorMatchState(State::Follower) || (followIndicatorMatchState(State::Leader) && state != State::Follower))) ||
            (state == State::Set2 && isPointedAt() != -1)) {
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
                setState(State::Set);
                if(inFlags[direction] != nullptr && inFlags[direction]->state == State::Seed){
                    setPoint((direction+1)%6);
                    headMarkDir = (direction+1)%6;
                    followDir = (direction+1)%6;
                }
                else if (inFlags[(direction+2)%6] != nullptr && (inFlags[(direction+2)%6]->state == State::Set || inFlags[(direction+2)%6]->state == State::Follower2)){
                    setPoint((direction+3)%6);
                    headMarkDir = (direction+3)%6;
                    followDir = (direction+3)%6;
                }
                else {
                    setPoint((direction+2)%6);
                    headMarkDir = (direction+2)%6;
                    followDir = (direction+2)%6;
                }
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
                setStopper();
            }
            if (inFlags[followDir] == nullptr && state == State::Follower2){
                setState(State::Leader2);
            }
            else if (inFlags[followDir] == nullptr && state == State::Seed){
                return Movement(MovementType::Idle);
            }
            else if (inFlags[followDir]->state == State::Finished){
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
            auto label = isPointedAt();
            if (hasNeighborInState(State::Leader) || hasNeighborInState(State::Leader2) || hasNeighborInState(State::Set)){
                if (hasNeighborInState(State::Set)){
                    setPoint(firstNeighborInState(State::Set));
                    headMarkDir = firstNeighborInState(State::Set);
                }
                else if (firstNeighborInState(State::Leader2) == headMarkDir && label != -1 && inFlags[label]->state == State::Follower2){
                    setState(State::Follower2);
                }
                return Movement(MovementType::Idle);
            }
            else if (nearStopper()){
                setState(State::Finished);
                head = true;
            }
            else {
                auto moveDir = getMoveDir();
                setContractDir(moveDir);
                headMarkDir = moveDir;
                setPoint(moveDir);
                return Movement(MovementType::Expand, moveDir);
            }
        }

        else if (state == State::Set){
            auto label = isPointedAt();
            if (label != -1 && (inFlags[label]->state == State::Follower2 || inFlags[label]->state == State::Seed || inFlags[label]->state == State::Leader2) && !hasNeighborInState(State::Follower) && !hasNeighborInState(State::Leader)){
                if (inFlags[headMarkDir] != nullptr && inFlags[label]->state != State::Leader2){
                    setState(State::Follower2);
                }
                else {
                    setState(State::Leader2);
                }
                setPoint(headMarkDir);
                if (inFlags[headMarkDir] == nullptr && inFlags[(headMarkDir+5)%6]->state != State::Follower2){
                    while (inFlags[headMarkDir] == nullptr){
                        headMarkDir = (headMarkDir+5)%6;
                    }
                }
            }
            return Movement(MovementType::Idle);
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
    else if(state == State::Set2) {
        headMarkColor = 0xaaaa00; tailMarkColor = 0xaaaa00; // Dark Yellow
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
bool Ring::allNeighborsInState(State _state){
    for (int label = 0; label < 5; label++){
        if (!neighborInState(label, _state)){
            return false;
        }
    }
    return true;
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
    else if (state == State::Set2){
        objectDir = firstNeighborInState(State::Set2);
        if (neighborInState(objectDir, State::Set2)){
            objectDir = (objectDir+1)%6;
        }
        if (neighborInState(objectDir, State::Set2)){
            objectDir = (objectDir+1)%6;
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

int Ring::unsetFollowIndicator()
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
