#include <deque>

#include "sim/system.h"

SystemIterator::SystemIterator(const System* system, int pos)
    : _pos(pos)
    , system(system)
{

}

bool SystemIterator::operator!=(const SystemIterator& other) const
{
    return _pos != other._pos;
}

const SystemIterator& SystemIterator::operator++()
{
    ++_pos;
    return *this;
}

const Particle& SystemIterator::operator*() const
{
    return system->at(_pos);
}

System::System()
{

}

System::~System()
{

}

SystemIterator System::begin() const
{
    return SystemIterator(this, 0);
}

SystemIterator System::end() const
{
    return SystemIterator(this, size());
}

int System::numMovements() const
{
    return 0;
}

int System::numRounds() const
{
    return 0;
}

bool System::hasTerminated() const
{
    return false;
}

bool System::isConnected(std::set<Node>& occupiedNodes)
{
    std::deque<Node> queue;
    queue.push_back(*occupiedNodes.begin());

    while(!queue.empty()) {
        Node n = queue.front();
        queue.pop_front();
        for(int dir = 0; dir < 6; ++dir) {
            Node neighbor = n.nodeInDir(dir);
            auto nondeIt = occupiedNodes.find(neighbor);
            if(nondeIt != occupiedNodes.end()) {
                queue.push_back(neighbor);
                occupiedNodes.erase(nondeIt);
            }
        }
    }

    return occupiedNodes.empty();
}
