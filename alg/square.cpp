#include <set>
#include <random>
#include <QTime>
#include "square.h"
#include "sim/particle.h"
#include "sim/system.h"
#include <QDebug>

namespace Square {
	SquareFlag::SquareFlag() 
		: point(false)
		  ,side(false)
		  ,followIndicator(false) 
	{
	}

	SquareFlag::SquareFlag(const SquareFlag& other) 
		: Flag(other)
		  ,state(other.state)
		  ,point(other.point)
		  ,side(other.side)
		  ,followIndicator(other.followIndicator)
	{
	}

	Square::Square(const State _state) 
		: state(_state)
		  ,followDir(-1)
	{
		if(_state == State::Seed) {
			outFlags[0].point = true;
			headMarkDir = 0;
		}
		setState(_state);
	}

	Square::Square(const Square& other)
		: AlgorithmWithFlags(other)
		  ,state(other.state)
		  ,followDir(other.followDir)
	{
	}

	Square::~Square() {
	}

	std::shared_ptr<System> Square::instance(const unsigned int size, const double holeProb) {
	    std::shared_ptr<System> system = std::make_shared<System>();
	    std::set<Node> occupied, candidates;

	    // Create Seed Particle
	    system->insert(Particle(std::make_shared<Square>(State::Seed), randDir(), Node(0,0), -1));
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
		system->insert(Particle(std::make_shared<Square>(State::Idle), randDir(), head, -1));
	    }
    return system;
	} 

	//logic for movement of particles
	Movement Square::execute() {

		//finish movement if particle is expanded
		if(isExpanded()) {
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
		
		//this statement gives particles their initial outward orientation
		if(state == State::Idle) {
			if(hasNeighborInState(State::Seed)) {
				auto label = firstNeighborInState(State::Seed);
				followDir = (labelToDir(label)+3)%6; //followdir is actual direction
				//qDebug() << "his orientation is: " << inFlags[labelToDir(label)]->headMarkDir;
				setFollowIndicatorLabel(followDir);
				headMarkDir = followDir; //headMarkDir is ui component
				setState(State::Leader);
				return Movement(MovementType::Idle);
			}
			else if(hasNeighborInState(State::Leader)) {
				auto label = firstNeighborInState(State::Leader);
				followDir = (labelToDir(label)+3)%6;
				setFollowIndicatorLabel(followDir);
				headMarkDir = followDir;
				setState(State::Leader);
				return Movement(MovementType::Idle);
			}
			else {
				
				return Movement(MovementType::Idle);
			}
		}

		//here we define specific movement based on number of neighbors
		else if(state == State::Leader) {
			switch(neighborCount()) {
				{case 0:
					qDebug() << "Something went wrong, I have no neighbors!";
					break;}
				//move CCW around neighbor to find another neighbor
				{case 1:
					//auto moveDir = getMoveDir();
	                //setContractDir(moveDir);
	                //headMarkDir = moveDir;
	                //return Movement(MovementType::Expand, moveDir);
headMarkColor = 0x66FF66; //light green
	                break;}
	            {case 2:
	            	if(inSafeFormation()) {
	            		//return Movement(MovementType::Idle);
	            	}
	            	else {
	            		//get out of unsafe formation
	            	}
headMarkColor = 0x33CC33; //medium light green	           
	            	break;}
	            {case 3:
	            	if(hasNeighborInState(State::Seed)) {
	            		//return Movement(MovementType::Idle);
	            	}
	            	else {
	            		//move outward to have only two neighbors
	            	}
headMarkColor = 0x009933; //medium green	            	
	            	break;}
	            {case 4:
headMarkColor = 0x003300; //dark green    
					break;}      
	            {case 5:
headMarkColor = 0x0D3310; //dark green          
					break;}	         
	            {case 6:
headMarkColor = 0x000000; //dark green          
	            	break;}
			}
			return Movement(MovementType::Idle);
		}



		else {
			return Movement(MovementType::Idle);
		}
		
	}

	std::shared_ptr<Algorithm> Square::clone() {
		return std::make_shared<Square>(*this);
	}

	bool Square::isDeterministic() const {
		return true; //not sure ifthis is true, haven't written alg yet
	}

	int Square::isPointedAt() {
	    for(int label = 0; label < 10; label++) {
	        if(inFlags[label] != nullptr) {
	            if(inFlags[label]->point) {
	                return label;
	            }
	        }
	    }
	    return -1;
	}

	void Square::setPoint(int _label){
	    for(int label = 0; label < 10; label++) {
	        outFlags[label].point = false;
	    }
	    if(_label != -1) {
	        outFlags[_label].point = true;
	    }
	}

	void Square::setState(State _state) {
	    state = _state;
	    if(state == State::Seed) {
	        headMarkColor = 0x66FFFF; tailMarkColor = 0x66FFFF; // Green
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
	    else{ // phase == Phase::Idle
	        headMarkColor = -1; tailMarkColor = -1; // No color
	    }
	    for(int i = 0; i < 10; i++) {
	        outFlags[i].state = state;
	    }
	}

	//returns number of neighbors particle currently has
	int Square::neighborCount() {
		int count = 0;
		for(int label = 0; label < 10; label++) {
			if(inFlags[label] != nullptr) {
				count++;
				/*if(inFlags[label]->isExpanded())
					count--;*/
			}
		}
		if(count == 6) { //particle is fully surrounded
			headMarkColor = 0xff69b4; 
		}
		return count;
	}

	//for particles with 2 neighbors, checks if local neighborhood is in defined "safe" formation
	//this means that my two neighbors are not neighbors themselves
	bool Square::inSafeFormation() {
		auto label = firstNeighbor();
		auto direction = labelToDir(label);
		if((inFlags[(direction+1)%6] != nullptr) || (inFlags[(direction+5)%6]) != nullptr) {
			return false;
		}  
		return true;
	}

	bool Square::neighborInState(int direction, State _state) {
	    Q_ASSERT(0 <= direction && direction <= 9);
	    return (inFlags[direction] != nullptr && inFlags[direction]->state == _state);
	}

	bool Square::hasNeighborInState(State _state) {
	    return (firstNeighborInState(_state) != -1);
	}

	int Square::firstNeighbor() {
	    for(int label = 0; label < 10; label++) {
	        if(inFlags[label] != nullptr) {
	            return label;
	        }
	    }
    	return -1; //no neighbors
	}

	int Square::firstNeighborInState(State _state) {
	    for(int label = 0; label < 10; label++) {
	        if(neighborInState(label, _state)) {
	            return label;
	        }
	    }
	    return -1;
	}

	int Square::getMoveDir() {
	    Q_ASSERT(isContracted());
	    int objectDir = firstNeighborInState(State::Leader);
	    if(hasNeighborInState(State::Seed)){
	        objectDir = firstNeighborInState(State::Seed);
	    }
	    objectDir = (objectDir+5)%6;
	    if(neighborInState(objectDir, State::Leader)){
	        objectDir = (objectDir+5)%6;
	    }
	    return labelToDir(objectDir);
	}

	void Square::setContractDir(const int contractDir) {
	    for(int label = 0; label < 10; label++) {
	        outFlags[label].contractDir = contractDir;
	    }
	}

	int Square::updatedFollowDir() const {
	    int contractDir = inFlags[followDir]->contractDir;
	    int offset = (followDir - inFlags[followDir]->dir + 9) % 6;
	    int tempFollowDir = (contractDir + offset) % 6;
	    Q_ASSERT(0 <= tempFollowDir && tempFollowDir <= 5);
	    return tempFollowDir;
	}

	void Square::unsetFollowIndicator() {
	    for(int i = 0; i < 10; i++) {
	        outFlags[i].followIndicator = false;
	    }
	}

	void Square::setFollowIndicatorLabel(const int label) {
	    for(int i = 0; i < 10; i++) {
	        outFlags[i].followIndicator = (i == label);
	    }
	}

	bool Square::tailReceivesFollowIndicator() const {
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

	bool Square::followIndicatorMatchState(State _state) const {
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
