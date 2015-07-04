/*
 * This is an algorithm to form a square, by Miles Laff
 * Based on other AmoebotSim algorithms by Brian Parker
 * */

#ifndef SQUARE_H
#define SQUARE_H

#include "alg/algorithmwithflags.h"

class System;

namespace Square 
{
	enum class State 
	{
		Finished
		,Compacted
		,Leader
		,Follower
		,Idle
		,Seed
		,CompactedLeader
		,CompactedFollower
	};

	//this extends Flag class, holds information shared with other particles
	class SquareFlag : public Flag 
	{
	public:
		SquareFlag();
		SquareFlag(const SquareFlag& other);
		State state;
		bool point;
		bool side; //side flag indicates right side of forming square
		bool followIndicator;
		int contractDir;
	};

	class Square : public AlgorithmWithFlags<SquareFlag>
	{
	public:
		Square(const State _state);
		Square(const Square& other);
		virtual ~Square();

		static System* instance(const unsigned int numParticles, const double holeProb);

		virtual Movement execute();
		virtual std::shared_ptr<Algorithm> clone() override;
		virtual bool isDeterministic() const;

	protected:
		int isPointedAt();
		void setPoint(int _label);
		void setState(State _state);
		int neighborCount();
		bool inSafeFormation();
		bool neighborInState(int direction, State _state); //implement pls
		bool hasNeighborInState(State _state);
		int firstNeighbor();
		int firstNeighborInState(State _state);

	    int getMoveDir();
	    void setContractDir(const int contractDir);
	    int updatedFollowDir() const;

	    void unsetFollowIndicator();
    	void setFollowIndicatorLabel(const int label);
    	bool tailReceivesFollowIndicator() const;
   		bool followIndicatorMatchState(State _state) const;

		State state;
		int followDir;
	};
}

#endif //SQUARE_H
