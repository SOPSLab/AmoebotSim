#ifndef UNIVERSALCOATINGHELPER
#define UNIVERSALCOATINGHELPER

#include <QDebug>

#include <deque>
#include <functional>
#include <set>
#include <unordered_map>

#include "sim/node.h"
#include "sim/particle.h"

// use this to get some debug output
//#define UNIVERSAL_COATING_HELPER_DEBUG
// additionally use this to output matchings
//#define UNIVERSAL_COATING_HELPER_DEBUG_MATCHING

namespace UniversalCoating
{

inline void setupObjectAndParticles(const System& system,
                                    std::set<Node>& object,
                                    std::set<Node>& particles)
{
    for(int i = 0; i < system.getNumParticles(); i++) {
        const Particle& p = system.at(i);
        if(p.isStatic()) {
            object.insert(p.head);
        } else {
            particles.insert(p.head);
        }
    }
}

inline int setupOpenPositions(const std::set<Node>& object,
                               const std::set<Node>& particles,
                               std::set<Node>& mandatoryOpenPositions,
                               std::set<Node>& optionalOpenPositions)
{
    int numMandatoryLayers = 0;
    std::set<Node> growingObject;
    growingObject.insert(object.cbegin(), object.cend());

    while(true) {
        // add another layer to the growingObject
        std::set<Node> layer;
        for(auto& node : growingObject) {
            for(int i = 0; i < 6; i++) {
                Node neighbor = node.nodeInDir(i);
                if(growingObject.find(neighbor) == growingObject.end()) {
                    layer.insert(neighbor);
                }
            }
        }

        if(mandatoryOpenPositions.size() + layer.size() <= particles.size()) {
            // found a layer of mandatory positions.
            mandatoryOpenPositions.insert(layer.cbegin(), layer.cend());
            growingObject.insert(layer.cbegin(), layer.cend());
            numMandatoryLayers++;
        } else {
            // the first layer that cannot be added to mandatoryOpenPositions is the set of optional open positions
            optionalOpenPositions = layer;
            // escape from while loop
            break;
        }
    }

    Q_ASSERT(mandatoryOpenPositions.size() <= particles.size());
    Q_ASSERT(mandatoryOpenPositions.size() + optionalOpenPositions.size() >= particles.size());

    return numMandatoryLayers;
}

inline int getWeakLowerBound(const System& system)
{
    std::set<Node> object, particles;
    setupObjectAndParticles(system, object, particles);

    std::set<Node> mandatoryOpenPositions, optionalOpenPositions;
    int numMandatoryLayers = setupOpenPositions(object, particles, mandatoryOpenPositions, optionalOpenPositions);

    const int numOptionalOpenPositionsToBeFilled = particles.size() - mandatoryOpenPositions.size();
    const int numOptionalOpenPositionsToRemain = optionalOpenPositions.size() - numOptionalOpenPositionsToBeFilled;
    int lowerBound = 0;

    for(auto& node : particles) {
        mandatoryOpenPositions.erase(node);
        optionalOpenPositions.erase(node);
    }

    std::set<Node> growingParticles;
    growingParticles.insert(particles.cbegin(), particles.cend());

    while(!mandatoryOpenPositions.empty() || (int) optionalOpenPositions.size() > numOptionalOpenPositionsToRemain) {
        // add another layer to the growingParticles
        std::set<Node> layer;
        for(auto& node : growingParticles) {
            for(int i = 0; i < 6; i++) {
                Node neighbor = node.nodeInDir(i);
                if(growingParticles.find(neighbor) == growingParticles.end() && object.find(neighbor) == object.end()) {
                    layer.insert(neighbor);
                }
            }
        }

        for(auto& node : layer) {
            mandatoryOpenPositions.erase(node);
            optionalOpenPositions.erase(node);
        }

        growingParticles.insert(layer.cbegin(), layer.cend());
        lowerBound++;
    }

    Q_ASSERT(mandatoryOpenPositions.empty());

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
    qDebug() << "-----------------------------------------";
    qDebug() << "numObjectPositions:       " << object.size();
    qDebug() << "numParticlePositions:     " << particles.size();
    qDebug() << "numMandatoryLayers:       " << numMandatoryLayers;
    qDebug() << "numMandatoryOpenPositions:" << mandatoryOpenPositions.size();
    qDebug() << "numOptionalOpenPositions: " << optionalOpenPositions.size();
    qDebug() << "lowerBound:               " << lowerBound;
    qDebug() << "-----------------------------------------";
#endif

    return lowerBound;
}

inline std::set<Node> nextLayer(const std::set<Node>& lastLayer, std::function<bool(Node)> checkFunc)
{
    std::set<Node> layer;
    for(const Node& node : lastLayer) {
        for(int i = 0; i < 6; i++) {
            const Node neighbor = node.nodeInDir(i);
            if(checkFunc(neighbor)) {
                layer.insert(neighbor);
            }
        }
    }
    return layer;
}

class Graph
{
private:
    struct GraphNode
    {
        GraphNode(Node node, bool right) : node(node), right(right), mate(nullptr), pred(nullptr) { }

        Node node;
        bool right;
        GraphNode* mate;
        GraphNode* pred;
        std::unordered_map<GraphNode*, int> distance;
    };

    struct Rect
    {
        Rect() :
            minX(std::numeric_limits<int>::max()),
            maxX(std::numeric_limits<int>::min()),
            minY(std::numeric_limits<int>::max()),
            maxY(std::numeric_limits<int>::min())
        { }

        void include(const Node& node)
        {
            if(node.x < minX) minX = node.x;
            if(node.x > maxX) maxX = node.x;
            if(node.y < minY) minY = node.y;
            if(node.y > maxY) maxY = node.y;
        }

        bool contains(const Node& node) const
        {
            return minX <= node.x && node.x <= maxX &&
                   minY <= node.y && node.y <= maxY;
        }

        int minX, maxX, minY, maxY;
    };

public:
    Graph(const std::set<Node>& object) : object(object) { }

    ~Graph()
    {
        for(GraphNode* node : nodes) {
            delete node;
        }
    }

    void addNode(Node node, bool right)
    {
        GraphNode* graphNode = new GraphNode(node, right);
        nodes.push_back(graphNode);

        // setup distances
        std::deque<GraphNode*> undiscoveredGraphNodes = nodes;

        std::set<Node> visitedNodes;
        visitedNodes.insert(graphNode->node);

        Rect boundingRect = getLooseBoundingRect();

        int distance = 0;
        std::set<Node> lastLayer;
        while(!undiscoveredGraphNodes.empty()) {
            // check whether a graph node was discovered and if so set the distance accordingly
            for(unsigned int i = 0; i < undiscoveredGraphNodes.size(); i++) {
                GraphNode* otherGraphNode = undiscoveredGraphNodes.at(i);
                if(visitedNodes.find(otherGraphNode->node) != visitedNodes.end()) {
                    graphNode->distance.insert(std::pair<GraphNode*, int>(otherGraphNode, distance));
                    otherGraphNode->distance.insert(std::pair<GraphNode*, int>(graphNode, distance));
                    std::swap(undiscoveredGraphNodes.at(i), undiscoveredGraphNodes.at(0));
                    undiscoveredGraphNodes.pop_front();
                }
            }

            // returns true if the node should be added to the layer
            auto checkFunc = [&] (const Node& node)
            {
                return  boundingRect.contains(node) &&
                        visitedNodes.find(node) == visitedNodes.end() &&
                        object.find(node) == object.end();
            };

            // add another layer to bfs
            std::set<Node> layer;
            if(distance == 0) {
                layer = nextLayer(visitedNodes, checkFunc);
            } else {
                layer = nextLayer(lastLayer, checkFunc);
            }

            visitedNodes.insert(layer.cbegin(), layer.cend());
            lastLayer.swap(layer);

            distance++;
        }
    }

    void updateMatching(const int weightLimit)
    {
        // augment as far as possible
        while(augment(weightLimit)) {
            Q_ASSERT(isValid());
        }

#ifdef UNIVERSAL_COATING_HELPER_DEBUG_MATCHING
        for(GraphNode* node : nodes) {
            if(!node->right && node->mate != nullptr) {
                qDebug() << "(" << node->node.x << ", " << node->node.y << ") <--> (" << node->mate->node.x << ", " << node->mate->node.y << ")";
            }
        }
#endif
    }

    void reset()
    {
        for(GraphNode* node : nodes) {
            node->mate = nullptr;
        }
    }

    int getMatchingSize()
    {
        int matchingSize = 0;
        for(GraphNode* node : nodes) {
            if(!node->right && node->mate != nullptr) {
                matchingSize++;
            }
        }
        return matchingSize;
    }

    int getMaxPairwiseDistance()
    {
        int max = -1;
        for(GraphNode* node : nodes) {
            for(auto pair : node->distance) {
                const int dist = pair.second;
                if(dist > max) {
                    max = dist;
                }
            }
        }
        return max;
    }

    std::deque<GraphNode*> getMates()
    {
        std::deque<GraphNode*> mates;
        for(GraphNode* node : nodes) {
            mates.push_back(node->mate);
        }
        return mates;
    }

    void setMates(const std::deque<GraphNode*>& mates)
    {
        std::deque<GraphNode*> matesCopy = mates;
        for(GraphNode* node : nodes) {
            if(matesCopy.empty()) {
                node->mate = nullptr;
            } else {
                node->mate = matesCopy.front();
                matesCopy.pop_front();
            }
        }
    }

private:
    bool isValid()
    {
        for(GraphNode* node : nodes) {
            if(node->mate != nullptr) {
                if(node->mate->mate != node && node->right == node->mate->right) {
                    return false;
                }
            }

            if(node->pred != nullptr) {
                return false;
            }
        }
        return true;
    }

    Rect getLooseBoundingRect()
    {
        Rect rect;
        for(GraphNode* node : nodes) {
            rect.include(node->node);
        }

        rect.minX = rect.minX - 1;
        rect.maxX = rect.maxX + 1;
        rect.minY = rect.minY - 1;
        rect.maxY = rect.maxY + 1;

        return rect;
    }

    bool areNeighbors(const int distanceLimit, GraphNode* node1, GraphNode* node2)
    {
        return (node1->right != node2->right) && (node1->distance.at(node2) <= distanceLimit);
    }

    void clean(std::deque<GraphNode*>& visitedNode)
    {
        for(GraphNode* node : visitedNode) {
            node->pred = nullptr;
        }
    }

    void augmentPath(GraphNode* node)
    {
        // fun!
        while(node != node->pred) {
            node->mate = node->pred;
            node->pred->mate = node;
            node = node->pred->pred;
        }
    }

    bool augment(const int distanceLimit)
    {
        std::deque<GraphNode*> unvisitedNodes = nodes;

        // look for augmenting path using bfs
        // bfs queue
        std::deque<GraphNode*> queue;

        // queue for cleanup
        std::deque<GraphNode*> visitedNodes;

        // add all unmatched node in left set to queue
        // the bfs is started from these nodes
        for(unsigned int i = 0; i < unvisitedNodes.size(); i++) {
            GraphNode* node = unvisitedNodes.at(i);
            if(!node->right && node->mate == nullptr) {
                node->pred = node;
                queue.push_back(node);
                visitedNodes.push_back(node);
                std::swap(unvisitedNodes[i], unvisitedNodes[0]);
                unvisitedNodes.pop_front();
            }
        }

        // bfs
        while(!queue.empty()) {
            GraphNode* node = queue.front();
            queue.pop_front();

            for(unsigned int i = 0; i < unvisitedNodes.size(); i++) {
                GraphNode* otherNode = unvisitedNodes.at(i);
                if(areNeighbors(distanceLimit, node, otherNode) && otherNode->pred == nullptr) {
                    otherNode->pred = node;
                    queue.push_back(otherNode);
                    visitedNodes.push_back(otherNode);
                    std::swap(unvisitedNodes[i], unvisitedNodes[0]);
                    unvisitedNodes.pop_front();

                    if(otherNode->right && otherNode->mate == nullptr) {
                        // augmenting path found
                        augmentPath(otherNode);
                        clean(visitedNodes);
                        return true;
                    }
                }
            }
        }

        clean(visitedNodes);
        return false;
    }

private:
    std::deque<GraphNode*> nodes;
    const std::set<Node>& object;
};

inline int binarySearch(const int start, std::function<bool(int)> func)
{
    if(func(0)) {
        return 0;
    }

    int value = start;
    int delta = start;
    while(delta >= 1) {
        if(func(value)) {
            value = value - delta;
        } else {
            value = value + delta;
        }
        delta = delta / 2;
    }

    if(!func(value)) {
        value = value + 1;
    }

    Q_ASSERT(func(value) && !func(value - 1));
    return value;
}

inline int getStrongLowerBound(const System& system)
{
    std::set<Node> object, particles;
    setupObjectAndParticles(system, object, particles);

    std::set<Node> mandatoryOpenPositions, optionalOpenPositions;
    int numMandatoryLayers = setupOpenPositions(object, particles, mandatoryOpenPositions, optionalOpenPositions);

    // setup graph
    Graph graph(object);
    for(auto& node : particles) {
        graph.addNode(node, false);
    }
    for(auto& node : mandatoryOpenPositions) {
        graph.addNode(node, true);
    }

    // The mandatory open positions have to be filled.
    // Hence, we first search for the minimal maximum distance such that there is a matching
    // between the particle positions and the mandatory open positions that covers all mandatory open positions.

    // setup maxPairwiseDistancePowerOfTwo to be the smallest power of two greater than graph.getMaxPairwiseDistance()
    const int maxPairwiseDistance = graph.getMaxPairwiseDistance();
    int maxPairwiseDistancePowerOfTwo = 1;
    while(maxPairwiseDistance > maxPairwiseDistancePowerOfTwo) {
        maxPairwiseDistancePowerOfTwo = 2 * maxPairwiseDistancePowerOfTwo;
    }

    // use binary search over the interval [0, maxPairwiseDistancePowerOfTwo]
    // to find minimal maximum distance such that there is a matching is as described above
    auto mandatoryFunc = [&](int distance)
    {
        graph.updateMatching(distance);
        bool success = (graph.getMatchingSize() == (int) mandatoryOpenPositions.size());
        graph.reset();
        return success;
    };
    int maxDistanceMandatory = binarySearch(maxPairwiseDistancePowerOfTwo, mandatoryFunc);

    // Now, we try to augment the resulting matching to find the minimal maximum distance such that there is a matching
    // between the particle positions and all open positions that covers at the same time:
    // a) all mandatory positions
    // b) all particle positions
    // We reuse the matching from the particle positions to the mandatory positions from above.
    // In this matching, all mandatory positions are matched.
    // Augmenting the matching cannot "unmatch" these nodes.

    // backup matching
    graph.updateMatching(maxDistanceMandatory);
    auto mates = graph.getMates();

    // extend graph by optional open positions
    for(auto& node : optionalOpenPositions) {
        graph.addNode(node, true);
    }

    // use binary search over the interval [maxDistanceMandatory, maxPairwiseDistancePowerOfTwo]
    // to find minimal maximum distance such that there is a matching is as described above
    auto optionalFunc = [&](int distance)
    {
        if(distance < maxDistanceMandatory) {
            return false;
        } else {
            graph.updateMatching(distance);
            bool success = (graph.getMatchingSize() == (int) particles.size());
            graph.setMates(mates);
            return success;
        }
    };
    int maxDistanceOptional = binarySearch(maxPairwiseDistancePowerOfTwo, optionalFunc);

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
    qDebug() << "-----------------------------------------";
    qDebug() << "numObjectPositions:       " << object.size();
    qDebug() << "numParticlePositions:     " << particles.size();
    qDebug() << "numMandatoryLayers:       " << numMandatoryLayers;
    qDebug() << "numMandatoryOpenPositions:" << mandatoryOpenPositions.size();
    qDebug() << "numOptionalOpenPositions: " << optionalOpenPositions.size();
    qDebug() << "maxPairwiseDistance:      " << maxPairwiseDistance;
    qDebug() << "maxDistanceMandatory:     " << maxDistanceMandatory;
    qDebug() << "maxDistanceOptional:      " << maxDistanceOptional;
    qDebug() << "-----------------------------------------";
#endif

    return maxDistanceOptional;
}

}

#endif // UNIVERSALCOATINGHELPER

