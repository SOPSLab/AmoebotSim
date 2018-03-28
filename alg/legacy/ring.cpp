//#include <set>
//#include <random>

//#include <QTime>

//#include "alg/legacy/legacyparticle.h"
//#include "alg/legacy/legacysystem.h"
//#include "alg/legacy/ring.h"

//namespace Ring
//{
//RingFlag::RingFlag()
//    : point(false),
//      stopper(false),
//      followIndicator(false)
//{
//}

//RingFlag::RingFlag(const RingFlag& other)
//    : Flag(other),
//      state(other.state),
//      point(other.point),
//      stopper(other.stopper),
//      followIndicator(other.followIndicator),
//      contractDir(other.contractDir)
//{
//}

//Ring::Ring(const State _state)
//    : state(_state),
//      followDir(-1),
//      wait(0)
//{
//    if (_state == State::Seed){
//        outFlags[0].point = true;
//        headMarkDir = 0;
//        followDir = 0;
//    }
//    setState(_state);
//}


//Ring::Ring(const Ring& other)
//    : AlgorithmWithFlags(other),
//      state(other.state),
//      followDir(other.followDir),
//      wait(other.wait)
//{
//}

//Ring::~Ring()
//{
//}

//std::shared_ptr<LegacySystem> Ring::instance(const unsigned int size, const double holeProb)
//{
//    std::shared_ptr<LegacySystem> system = std::make_shared<LegacySystem>();
//    std::set<Node> occupied, candidates;

//    // Create Seed Particle
//    system->insertParticle(LegacyParticle(std::make_shared<Ring>(State::Seed), randDir(), Node(0,0), -1));
//    occupied.insert(Node(0,0));

//    for(int dir = 0; dir<6;dir++){
//        candidates.insert(Node(0,0).nodeInDir(dir));
//    }

//    while(occupied.size() < size && !candidates.empty()){
//        auto index = randInt(0, candidates.size());
//        auto it = candidates.begin();
//        while (index != 0){
//            ++it;
//            index--;
//        }

//        Node head = *it;
//        candidates.erase(it);
//        occupied.insert(head);

//        if(randBool(holeProb)){
//            continue;
//        }

//        for(int dir = 0; dir < 6; dir++){
//            auto neighbor = head.nodeInDir(dir);
//            if(occupied.find(neighbor) == occupied.end() && candidates.find(neighbor) == candidates.end()){
//                candidates.insert(neighbor);
//            }
//        }
//        // Insert new idle particle
//        system->insertParticle(LegacyParticle(std::make_shared<Ring>(State::Idle), randDir(), head, -1));
//    }
//    return system;
//}

//std::shared_ptr<Algorithm> Ring::blank() const
//{
//    return std::make_shared<Ring>(State::Idle);
//}

//std::shared_ptr<Algorithm> Ring::clone()
//{
//    return std::make_shared<Ring>(*this);
//}

//bool Ring::isDeterministic() const
//{
//    return true;
//}

//bool Ring::isStatic() const
//{
//    return false;
//}
//bool Ring::isRetired() const
//{
//    return false;
//}


//Movement Ring::execute()
//{
//    if(isExpanded()){
//        if(state == State::Follower) {
//            setFollowIndicatorLabel(followDir);
//        }
//        // Sorry, long exception statement needed its own 3 lines
//        if  (hasNeighborInState(State::Idle) ||
//            state == State::Leader2 || state == State::Follower2 ||
//            (tailReceivesFollowIndicator() && (followIndicatorMatchState(State::Follower) || (followIndicatorMatchState(State::Leader) && state != State::Follower)))) {
//            return Movement(MovementType::HandoverContract, tailContractionLabel());
//        }
//        else {
//            return Movement(MovementType::Contract, tailContractionLabel());
//        }
//    }
//    else {
//        if (state == State::Idle){
//            if (hasNeighborInState(State::Set) || hasNeighborInState(State::Seed)){
//                setState(State::Leader);
//                return Movement(MovementType::Idle);
//            }
//            auto label = std::max(firstNeighborInState(State::Follower), firstNeighborInState(State::Leader));
//            if(label != -1) {
//                setState(State::Follower);
//                followDir = labelToDir(label);
//                setFollowIndicatorLabel(followDir);
//                headMarkDir = followDir;
//                return Movement(MovementType::Idle);
//            }
//        }

//        else if (state == State::Follower && !hasNeighborInState(State::Idle)) {
//            if (hasNeighborInState(State::Set)){
//                setState(State::Leader);
//            }
//            else {
//                if(inFlags[followDir] == nullptr){
//                    if (hasNeighborInState(State::Leader)){
//                        followDir = firstNeighborInState(State::Leader);
//                    }
//                    else {
//                        followDir = firstNeighborInState(State::Follower);
//                    }
//                }
//                if(inFlags[followDir]->isExpanded() && !inFlags[followDir]->fromHead) {
//                    int expansionDir = followDir;
//                    setContractDir(expansionDir);
//                    followDir = updatedFollowDir();
//                    headMarkDir = followDir;
//                    auto temp = dirToHeadLabelAfterExpansion(followDir, expansionDir);
//                    Q_ASSERT(labelToDirAfterExpansion(temp, expansionDir) == followDir);
//                    setFollowIndicatorLabel(temp);
//                    return Movement(MovementType::Expand, expansionDir);
//                }
//            }
//        }

//        else if (state == State::Leader && !hasNeighborInState(State::Idle)){
//            headMarkDir = -1;
//            int direction = isPointedAt();
//            if(direction != -1){
//                setState(State::Set); // Normal hexagon formation
//                if(neighborInState(direction, State::Seed)){
//                    direction = (direction+1)%6;
//                }
//                else if (neighborInState((direction+2)%6, State::Set) || neighborInState((direction+2)%6, State::Follower2)){
//                    direction = (direction+3)%6;
//                }
//                else {
//                    direction = (direction+2)%6;
//                }
//                setPoint(direction);
//                headMarkDir = direction;
//                followDir = direction;
//            }
//            else {
//                auto moveDir = getMoveDir();
//                setContractDir(moveDir);
//                headMarkDir = moveDir;
//                return Movement(MovementType::Expand, moveDir);
//            }
//        }

//        else if ((state == State::Follower2 || state == State::Seed) && !hasNeighborInState(State::Idle)){
//            if (hasNeighborInState(State::Leader2) && hasNeighborInState(State::Seed)){
//                setStopper(); // Set Stopper indicator
//            }
//            if (inFlags[followDir] == nullptr && state == State::Seed){
//                return Movement(MovementType::Idle);
//            }
//            else if (neighborInState(followDir, State::Finished)){
//                setState(State::Finished);
//            }
//            else {
//                if(inFlags[followDir]->isExpanded() && !inFlags[followDir]->fromHead) {
//                    int expansionDir = followDir;
//                    setContractDir(expansionDir);
//                    followDir = updatedFollowDir();
//                    headMarkDir = followDir;
//                    auto temp = dirToHeadLabelAfterExpansion(followDir, expansionDir);
//                    Q_ASSERT(labelToDirAfterExpansion(temp, expansionDir) == followDir);
//                    setFollowIndicatorLabel(temp);
//                    setPoint(expansionDir);
//                    return Movement(MovementType::Expand, expansionDir);
//                }
//            }
//        }

//        else if (state == State::Leader2){
//            if (nearStopper()){ // Checks for stopper particle (in mid section)
//                setState(State::Finished);
//            }
//            else {
//                auto moveDir = getMoveDir();
//                setContractDir(moveDir);
//                headMarkDir = moveDir;
//                setPoint(moveDir);
//                return Movement(MovementType::Expand, moveDir);
//            }
//        }

//        else if (state == State::Set && !hasNeighborInState(State::Leader) && !hasNeighborInState(State::Follower)){
//            auto label = isPointedAt();
//            if  (neighborInState(label,State::Follower2) || neighborInState(label,State::Seed)){
//                if (wait < 5) {
//                    wait++; // inclusion of a constant wait period is a rudimentary way to solve the issue of having some particles still being in hex formation
//                }
//                else {
//                    if (inFlags[headMarkDir] != nullptr){
//                        setState(State::Follower2);
//                    }
//                    else {
//                        setState(State::Leader2);
//                    }
//                    setPoint(headMarkDir);
//                }
//            }
//        }

//        else if (state == State::Finished){
//            return Movement(MovementType::Empty);
//        }
        
//    }
//    return Movement(MovementType::Idle);
//}

//int Ring::isPointedAt(){
//    for(int label = 0; label < 10; label++) {
//        if(inFlags[label] != nullptr) {
//            if(inFlags[label]->point){
//                return label;
//            }
//        }
//    }
//    return -1;
//}

//void Ring::setPoint(int _label){
//    for(int label = 0; label < 10; label++) {
//        outFlags[label].point = false;
//    }
//    if (_label != -1) {
//        outFlags[_label].point = true;
//    }
//}

//bool Ring::nearStopper(){
//    for (int label = 0; label < 10; label++){
//        if (inFlags[label] != nullptr && inFlags[label]->stopper){
//            return true;
//        }
//    }
//    return false;
//}
//void Ring::setStopper(bool value){
//    for (int label = 0; label < 10; ++label)
//    {
//        outFlags[label].stopper = value;
//    }
//}

//void Ring::setState(State _state)
//{
//    state = _state;
//    if (state == State::Seed){
//      headMarkColor = 0x00ff00; tailMarkColor = 0x00ff00; // Green
//    }
//    else if(state == State::Finished) {
//        headMarkColor = 0x000000; tailMarkColor = 0x000000; // Black
//    }
//    else if(state == State::Leader) {
//        headMarkColor = 0xff0000; tailMarkColor = 0xff0000; // Red
//    }
//    else if(state == State::Follower) {
//        headMarkColor = 0x0000ff; tailMarkColor = 0x0000ff; // Blue
//    }
//    else if(state == State::Set) {
//        headMarkColor = 0x777777; tailMarkColor = 0x777777; // Gray
//    }
//    else if(state == State::Leader2) {
//        headMarkColor = 0xaa0000; tailMarkColor = 0xaa0000; // Dark Red
//    }
//    else if(state == State::Follower2) {
//        headMarkColor = 0x0000aa; tailMarkColor = 0x0000aa; // Dark Blue
//    }
//    else { // phase == Phase::Idle
//        headMarkColor = -1; tailMarkColor = -1; // No color
//    }
//    for(int i = 0; i < 10; ++i) {
//        outFlags[i].state = state;
//    }
//}

//bool Ring::neighborInState(int direction, State _state){
//    Q_ASSERT(0 <= direction && direction <= 9);
//    return (inFlags[direction] != nullptr && inFlags[direction]->state == _state);
//}

//bool Ring::hasNeighborInState(State _state)
//{
//    return (firstNeighborInState(_state) != -1);
//}

//int Ring::firstNeighborInState(State _state)
//{
//    for(int label = 0; label < 10; label++) {
//        if(neighborInState(label, _state)) {
//            return label;
//        }
//    }
//    return -1;
//}

//int Ring::getMoveDir()
//{
//    Q_ASSERT(isContracted());
//    int objectDir = -1;
//    if (state == State::Leader){
//        objectDir = firstNeighborInState(State::Set);
//        if (hasNeighborInState(State::Seed)){
//            objectDir = firstNeighborInState(State::Seed);
//        }
//        if (hasNeighborInState(State::Follower2)){
//            objectDir = firstNeighborInState(State::Follower2);
//        }
//        objectDir = (objectDir+5)%6;
//        if (neighborInState(objectDir, State::Set) || neighborInState(objectDir, State::Follower2)){
//            objectDir = (objectDir+5)%6;
//        }
//    }
//    else {
//        objectDir = (firstNeighborInState(State::Follower2)+1)%6;
//        if (neighborInState(objectDir, State::Finished)){
//            objectDir = (objectDir+1)%6;
//        }
//        if (neighborInState(objectDir, State::Follower2)){
//            objectDir = (objectDir+1)%6;
//        }
//        if (neighborInState(objectDir, State::Follower2)){
//            objectDir = (objectDir+1)%6;
//        }
//    }
//    return labelToDir(objectDir);
//}

//void Ring::setContractDir(const int contractDir)
//{
//    for(int label = 0; label < 10; label++) {
//        outFlags[label].contractDir = contractDir;
//    }
//}
//int Ring::updatedFollowDir() const
//{
//    int contractDir = inFlags[followDir]->contractDir;
//    int offset = (followDir - inFlags[followDir]->dir + 9) % 6;
//    int tempFollowDir = (contractDir + offset) % 6;
//    Q_ASSERT(0 <= tempFollowDir && tempFollowDir <= 5);
//    return tempFollowDir;
//}

//void Ring::unsetFollowIndicator()
//{
//    for(int i = 0; i < 10; i++) {
//        outFlags[i].followIndicator = false;
//    }
//}

//void Ring::setFollowIndicatorLabel(const int label)
//{
//    for(int i = 0; i < 10; i++) {
//        outFlags[i].followIndicator = (i == label);
//    }
//}

//bool Ring::tailReceivesFollowIndicator() const
//{
//    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
//        auto label = *it;
//        if(inFlags[label] != nullptr) {
//            if(inFlags[label]->followIndicator) {
//                return true;
//            }
//        }
//    }
//    return false;
//}

//bool Ring::followIndicatorMatchState(State _state) const
//{
//    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
//        auto label = *it;
//        if(inFlags[label] != nullptr) {
//            if(inFlags[label]->followIndicator && inFlags[label]->state == _state) {
//                return true;
//            }
//        }
//    }
//    return false;
//}
//}

#include <QtGlobal>

#include <set>
#include <vector>

#include "alg/legacy/ring.h"

RingParticle::RingParticle(const Node head, const int globalTailDir,
                                 const int orientation, AmoebotSystem& system,
                                 State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    constructionDir(-1),
    moveDir(-1),
    followDir(-1) {
  totalNumber = -1;
  moveNum = -1;
  if (state == State::Seed) {
      constructionDir = 0;
      totalNumber = 1;
  }
}

void RingParticle::activate() {
  if (isExpanded()) {
    if (state == State::Follow) {
      if (!hasNbrInState({State::Idle}) && !hasTailFollower()) {
        contractTail();
      }
      return;
    } else if (state == State::Lead) {
      if (!hasNbrInState({State::Idle}) && !hasTailFollower()) {
        contractTail();
        updateMoveDir();
      }
      return;
    } else {
      Q_ASSERT(false);
    }
  } else {
    if (state == State::Seed) {
      if (hasNbrAtLabel(constructionDir) &&
          nbrAtLabel(constructionDir).state == State::Follow &&
          nbrAtLabel(constructionDir).totalNumber > 0) {
        state = State::Follow;
        followDir = constructionDir;
        return;
      }
      return;
    } else if (state == State::Idle) {
      if (hasNbrInState({State::Seed, State::Wait})) {
        state = State::Lead;
        updateMoveDir();
        return;
      } else if (hasNbrInState({State::Lead, State::Follow})) {
        state = State::Follow;
        followDir = labelOfFirstNbrInState({State::Lead, State::Follow});
        return;
      }
    } else if (state == State::Follow) {
      if (hasNbrInState({State::Seed, State::Wait}) && totalNumber < 0) {
        state = State::Lead;
        updateMoveDir();
        return;
      } else if (hasNbrInState({State::Finish}) &&
                 labelOfFirstNbrInState({State::Finish}, followDir)
                 == followDir) {
        state = State::Lead;
        moveDir = followDir;
        return;
      } else if (hasTailAtLabel(followDir)) {
        auto neighbor = nbrAtLabel(followDir);
        int neighborContractionDir =
          nbrDirToDir(neighbor, (neighbor.tailDir() + 3) % 6);
        push(followDir);
        followDir = neighborContractionDir;
        return;
      }
    } else if (state == State::Lead) {
      if (canWait() && totalNumber < 0) {
        state = State::Wait;
        updateConstructionDir();
        return;
      } else if (canFinish()) {
        state = State::Finish;
        updateConstructionDir();
        return;
      } else if (totalNumber > 0 && moveDir >= 0 && hasNbrAtLabel(moveDir)) {
        if (nbrAtLabel(moveDir).totalNumber < 0) {
          nbrAtLabel(moveDir).totalNumber = totalNumber + 1;
          nbrAtLabel(moveDir).moveNum = totalNumber / 6;
          nbrAtLabel(moveDir).state = State::Lead;
          state = State::Follow;
        }
        else if (nbrAtLabel(moveDir).state == State::Follow ||
                 nbrAtLabel(moveDir).state == State::Finish){
          state = State::Finish;
          updateConstructionDir();
          return;
        }
      } else {
        updateMoveDir();
        if(state == State::Finish) return;
        if (!hasNbrAtLabel(moveDir)) {
          moveNum++;
          expand(moveDir);
        } else if (hasTailAtLabel(moveDir)) {
          push(moveDir);
        }
        return;
      }
    } else if (state == State::Wait) {
      if (nbrAtLabel(constructionWaitReceiveDir()).totalNumber > 0
          && totalNumber < 0) {
        int dir = constructionWaitReceiveDir();
        if (dir > -1) {
          totalNumber = nbrAtLabel(dir).totalNumber + 1;
        } else Q_ASSERT(dir <= -1);
        moveNum = totalNumber;
        return;
      }
      if (!hasNbrAtLabel(constructionDir) && totalNumber > 0) {
        if (moveNum >= 0) {
          moveNum--;
          return;
        }
        state = State::Lead;
        moveDir = constructionDir;
        updateMoveDir();
        return;
      }
      if (totalNumber > 0 && hasNbrInState({State::Lead, State::Follow})) {
        int label = labelOfFirstNbrInState({State::Lead, State::Follow},
                                           constructionDir);
        if (label == constructionDir &&
            nbrAtLabel(label).totalNumber > 0) {
          state = State::Follow;
          followDir = labelOfFirstNbrInState({State::Lead, State::Follow},
                                             constructionDir);
          return;
        }
        return;
      }
      return;
    }
  }
}

int RingParticle::headMarkColor() const {
  switch(state) {
  case State::Seed:   return 0x00ff00;
  case State::Idle:   return -1;
  case State::Follow: return 0x0000ff;
  case State::Lead:   return 0xff0000;
  case State::Finish: return 0x000000;
  case State::Wait:   if (totalNumber < 0) return 0xffff00;
                      else                 return 0xff00ff;
  }

  return -1;
}

int RingParticle::headMarkDir() const {
  if (state == State::Lead) {
    return moveDir;
  } else if (state == State::Seed || state == State::Finish ||
             state == State::Wait) {
    return constructionDir;
  } else if (state == State::Follow) {
    return followDir;
  }
  return -1;
}

int RingParticle::tailMarkColor() const {
  return headMarkColor();
}

QString RingParticle::inspectionText() const {
  QString text;
  text += "head: (" + QString::number(head.x) + ", " + QString::number(head.y) +
    ")\n";
  text += "orientation: " + QString::number(orientation) + "\n";
  text += "globalTailDir: " + QString::number(globalTailDir) + "\n";
  text += "state: ";
  text += [this](){
    switch(state) {
    case State::Seed:   return "seed";
    case State::Idle:   return "idle";
    case State::Follow: return "follow";
    case State::Lead:   return "lead";
    case State::Finish: return "finish";
    case State::Wait:   return "wait";
    default:            return "no state";
    }
  }();
  text += "\n";
  text += "followDir: " + QString::number(followDir) + "\n";
  text += "moveDir: " + QString::number(moveDir) + "\n";
  text += "constructionDir: " + QString::number(constructionDir) + "\n";
  text += "totalNumber: " + QString::number(totalNumber) + "\n";
  text += "moveNum: " + QString::number(moveNum) + "\n";

  return text;
}

RingParticle& RingParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<RingParticle>(label);
}

int RingParticle::labelOfFirstNbrInState(std::initializer_list<State> states,
                                            int startLabel) const {
  auto propertyCheck = [&](const RingParticle& p) {
    for (auto state : states) {
      if (p.state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<RingParticle>(propertyCheck,
                                                           startLabel);
}

bool RingParticle::hasNbrInState(std::initializer_list<State> states)
  const {
  return labelOfFirstNbrInState(states) != -1;
}

int RingParticle::constructionFinishReceiveDir() const {
  auto propertyCheck = [&](const RingParticle& p) {
    return isContracted() &&
           (p.state == State::Seed || p.state == State::Finish) &&
           pointsAtMe(p, p.constructionDir);
  };

  return labelOfFirstNbrWithProperty<RingParticle>(propertyCheck);
}

int RingParticle::constructionWaitReceiveDir() const {
  auto propertyCheck = [&](const RingParticle& p) {
    return isContracted() &&
           (p.state == State::Seed || p.state == State::Wait) &&
           pointsAtMe(p, p.constructionDir);
  };

  return labelOfFirstNbrWithProperty<RingParticle>(propertyCheck);
}

bool RingParticle::canFinish() const {
  return constructionFinishReceiveDir() != -1;
}

bool RingParticle::canWait() const {
  return constructionWaitReceiveDir() != -1;
}

void RingParticle::updateConstructionDir() {
  if (state == State::Wait) {
    constructionDir = constructionWaitReceiveDir();
    if (nbrAtLabel(constructionDir).state == State::Seed) {
      constructionDir = (constructionDir + 1) % 6;
    } else {
      constructionDir = (constructionDir + 2) % 6;
    }

    if (hasNbrAtLabel(constructionDir) &&
        nbrAtLabel(constructionDir).state == State::Wait) {
      constructionDir = (constructionDir + 1) % 6;
    }
  } else if(state == State::Finish) {
    constructionDir = (moveDir + 3) % 6;
    while (!hasNbrAtLabel(constructionDir)) {
      constructionDir = (constructionDir + 1) % 6;
      if (hasNbrAtLabel(constructionDir) &&
          nbrAtLabel(constructionDir).state == State::Finish) {
        constructionDir = (constructionDir + 1) % 6;
      }
    }
  }
}

void RingParticle::updateMoveDir() {
  if (totalNumber < 0) {
    moveDir = labelOfFirstNbrInState({State::Seed, State::Wait});
    while (hasNbrAtLabel(moveDir) && (nbrAtLabel(moveDir).state == State::Seed
                                      ||
                                      nbrAtLabel(moveDir).state == State::Wait))
    {
      moveDir = (moveDir + 5) % 6;
    }
  }
  else {
    if (!hasNbrAtLabel((moveDir + 5) % 6)) {
      moveDir = (moveDir + 5) % 6;
      moveNum = 0;
    }
    if (moveNum == 2 * (totalNumber / 6) / 3
        && !isExpanded()) {
      state = State::Finish;
      constructionDir = (moveDir + 3) % 6;
    }
  }
}

bool RingParticle::hasTailFollower() const {
  auto propertyCheck = [&](const RingParticle& p) {
    return p.state == State::Follow &&
           pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
  };

  return labelOfFirstNbrWithProperty<RingParticle>(propertyCheck) != -1;
}

RingSystem::RingSystem(int numParticles, float holeProb) {
  Q_ASSERT(numParticles > 0);
  Q_ASSERT(0 <= holeProb && holeProb <= 1);

  // Insert the seed at (0,0).
  insert(new RingParticle(Node(0, 0), -1, randDir(), *this,
                             RingParticle::State::Seed));
  std::set<Node> occupied;
  occupied.insert(Node(0, 0));

  std::set<Node> candidates;
  for (int i = 0; i < 6; ++i) {
    candidates.insert(Node(0, 0).nodeInDir(i));
  }

  // Add inactive particles.
  int numNonStaticParticles = 0;
  while (numNonStaticParticles < numParticles && !candidates.empty()) {
    // Pick random candidate.
    int randIndex = randInt(0, candidates.size());
    Node randomCandidate;
    for (auto it = candidates.begin(); it != candidates.end(); ++it) {
      if (randIndex == 0) {
        randomCandidate = *it;
        candidates.erase(it);
        break;
      } else {
        randIndex--;
      }
    }

    occupied.insert(randomCandidate);

    // Add this candidate as a particle if not a hole.
    if (randBool(1.0f - holeProb)) {
      insert(new RingParticle(randomCandidate, -1, randDir(), *this,
                                 RingParticle::State::Idle));
      ++numNonStaticParticles;

      // Add new candidates.
      for (int i = 0; i < 6; ++i) {
        auto neighbor = randomCandidate.nodeInDir(i);
        if (occupied.find(neighbor) == occupied.end()) {
          candidates.insert(neighbor);
        }
      }
    }
  }
}

bool RingSystem::hasTerminated() const {
  #ifdef QT_DEBUG
    if (!isConnected(particles)) {
      return true;
    }
  #endif

  for (auto p : particles) {
    auto hp = dynamic_cast<RingParticle*>(p);
    if (hp->state != RingParticle::State::Seed &&
        hp->state != RingParticle::State::Finish) {
      return false;
    }
  }

  return true;
}
