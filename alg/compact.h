/* FILENAME:    compact.h
 * AUTHOR:      Josh Daymude, borrowing heavily from the structure and algorithms written by Brian Parker
 * DESCRIPTION: Header file for compact.cpp
*/

#ifndef COMPACT_H
#define COMPACT_H

#include "alg/algorithmwithflags.h"

class System;

namespace Compact
{
    enum class State {
        Finished,
        Compact,
        NotCompact,
        Leader,
        Follower,
        Active,
        Idle,
        Seed
    };

    // The pointer is initialized here in the HexFlag class.note the use of namespaces and classnames throughout.
    class CompactFlag : public Flag
    {
    public:
        CompactFlag();
        CompactFlag(const CompactFlag& other);
        State state;
        bool point;
        bool followIndicator;
        bool passingRoot;
        int contractDir;
        int nextFollowDirOffset;
    };

    class Compact : public AlgorithmWithFlags<CompactFlag>
    {
    public:
        Compact(const State _state);
        Compact(const Compact& other);
        virtual ~Compact();

        static System* instance(const unsigned int size, const double holeProb);

        virtual Movement execute();
        virtual Algorithm* clone();
        virtual bool isDeterministic() const;

    protected:
        int isPointedAt(); // Function to easily use the pointer mechanic

        void setState(State _state);
        bool neighborInState(int direction, State _state);
        bool hasNeighborInState(State _state);
        int firstNeighborInState(State _state);

        int getMoveDir();
        void setContractDir(const int contractDir);
        int updatedFollowDir() const;

        void unsetFollowIndicator();
        void setFollowIndicatorLabel(const int label);
        bool tailReceivesFollowIndicator() const;
        bool followIndicatorMatchState(State _state) const;

        // Things I've added:
        bool receivesFollowIndicator() const;
        int neighborCount();
        int adjacentNeighborCount();
        bool isLocallyCompact();
        int firstEmptyNeighbor();
        void setNextFollowDirOffset(const int currentFollowDir, const int expandDir);

    protected:
        State state;
        int followDir;
        const int dirAfterExpansionTable[6][6] = {{0,8,0,0,0,2}, {3,1,1,1,1,3}, {4,4,4,2,2,4}, {5,5,7,5,3,5}, {6,6,8,8,6,6}, {7,7,9,9,9,9}};
    };
}

#endif // COMPACT_H