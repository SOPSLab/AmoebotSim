#include <deque>

#include "sim/system.h"

System::System()
{

}

System::~System()
{

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
