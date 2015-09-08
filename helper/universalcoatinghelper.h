#ifndef UNIVERSALCOATINGHELPER
#define UNIVERSALCOATINGHELPER

#include <QDebug>

#include <deque>
#include <map>
#include <set>

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

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
    qDebug() << "numObjectPositions:" << object.size();
    qDebug() << "numParticlePositions:" << particles.size();
#endif
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

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
    qDebug() << "numMandatoryLayers:" << numMandatoryLayers;
    qDebug() << "numMandatoryOpenPositions:" << mandatoryOpenPositions.size();
    qDebug() << "numOptionalOpenPositions:" << optionalOpenPositions.size();
#endif

    Q_ASSERT(mandatoryOpenPositions.size() <= particles.size());
    Q_ASSERT(mandatoryOpenPositions.size() + optionalOpenPositions.size() >= particles.size());

    return numMandatoryLayers;
}

inline int getWeakLowerBound(const System& system)
{
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
    qDebug() << "------------------------------------";
#endif

    std::set<Node> object, particles;
    setupObjectAndParticles(system, object, particles);

    std::set<Node> mandatoryOpenPositions, optionalOpenPositions;
    setupOpenPositions(object, particles, mandatoryOpenPositions, optionalOpenPositions);

    const int numOptionalOpenPositionsToBeFilled = particles.size() - mandatoryOpenPositions.size();
    const int numOptinalOpenPositionsToRemain = optionalOpenPositions.size() - numOptionalOpenPositionsToBeFilled;
    int lowerBound = 0;

    for(auto& node : particles) {
        mandatoryOpenPositions.erase(node);
        optionalOpenPositions.erase(node);
    }

    std::set<Node> growingParticles;
    growingParticles.insert(particles.cbegin(), particles.cend());

    while(!mandatoryOpenPositions.empty() || (int) optionalOpenPositions.size() > numOptinalOpenPositionsToRemain) {
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
    qDebug() << "lowerBound:" << lowerBound;
    qDebug() << "------------------------------------";
#endif

    return lowerBound;
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
        std::map<GraphNode*, int> distance;
    };

    struct Rect
    {
        Rect() : minX(std::numeric_limits<int>::max()),
            maxX(std::numeric_limits<int>::min()),
            minY(std::numeric_limits<int>::max()),
            maxY(std::numeric_limits<int>::min())
        { }

        void include(const Node& node)
        {
            if(node.x < minX) {
                minX = node.x;
            }
            if(node.x > maxX) {
                maxX = node.x;
            }
            if(node.y < minY) {
                minY = node.y;
            }
            if(node.y > maxY) {
                maxY = node.y;
            }
        }

        bool isInside(const Node& node) const
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

            // add another layer to bfs
            std::set<Node> layer;
            for(const Node& node : visitedNodes) {
                for(int i = 0; i < 6; i++) {
                    Node neighbor = node.nodeInDir(i);
                    if(     boundingRect.isInside(neighbor) &&
                            visitedNodes.find(neighbor) == visitedNodes.end() &&
                            object.find(neighbor) == object.end()) {
                        layer.insert(neighbor);
                    }
                }
            }

            visitedNodes.insert(layer.cbegin(), layer.cend());
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

inline int getStrongLowerBound(const System& system)
{
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
    qDebug() << "------------------------------------";
#endif

    std::set<Node> object, particles;
    setupObjectAndParticles(system, object, particles);

    std::set<Node> mandatoryOpenPositions, optionalOpenPositions;
    setupOpenPositions(object, particles, mandatoryOpenPositions, optionalOpenPositions);

    if(mandatoryOpenPositions.size() == 0) {
        // no layer is mandatory
        // the particles have to form a connected snake on the surface
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
        qDebug() << "there is no completely filled layer";
        qDebug() << "sorry, this isn't implemented, yet";
#endif
        return -1;
    } else {
        // there will be at least one completely filled layer around the object
        // the particles do not have to form a connected snake in the last, incomplete layer

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
        qDebug() << "at least one complete layer will be filled";
        qDebug() << "setting up graph";
#endif

        Graph graph(object);

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
        qDebug() << "adding particles";
#endif
        for(auto& node : particles) {
            graph.addNode(node, false);
        }

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
        qDebug() << "adding mandatory open positions";
#endif
        for(auto& node : mandatoryOpenPositions) {
            graph.addNode(node, true);
        }

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
        qDebug() << "searching for minimal maximum distance for mandatory open positions";
#endif

        // The mandatory open positions have to be filled.
        // Hence, we first search for the minimal maximum distance such that there is a matching
        // between the particle positions and the mandatory open positions that covers all mandatory open positions.

        // setup maxPairwiseDistancePowerOfTwo to be the smallest power of two greater than graph.getMaxPairwiseDistance()
        const int maxPairwiseDistance = graph.getMaxPairwiseDistance();
        int maxPairwiseDistancePowerOfTwo = 1;
        while (maxPairwiseDistance > maxPairwiseDistancePowerOfTwo) {
            maxPairwiseDistancePowerOfTwo = 2 * maxPairwiseDistancePowerOfTwo;
        }

        // use binary search in the interval [0, maxPairwiseDistancePowerOfTwo] to find minimal maximum distance
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
        qDebug() << "searching maxDistanceMandatory";
#endif
        int maxDistanceMandatory = maxPairwiseDistancePowerOfTwo;
        int delta = maxDistanceMandatory;
        while(delta >= 1) {
            graph.updateMatching(maxDistanceMandatory);
            if(graph.getMatchingSize() == (int) mandatoryOpenPositions.size()) {
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
                qDebug() << maxDistanceMandatory << " --> success";
#endif
                maxDistanceMandatory = maxDistanceMandatory - delta;
                delta = delta / 2;
            } else {
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
                qDebug() << maxDistanceMandatory << " --> failure";
#endif
                maxDistanceMandatory = maxDistanceMandatory + delta;
                delta = delta / 2;
            }
            graph.reset();
        }

        graph.updateMatching(maxDistanceMandatory);
        if(graph.getMatchingSize() == (int) mandatoryOpenPositions.size()) {
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
            qDebug() << maxDistanceMandatory << " --> success";
#endif
        } else {
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
            qDebug() << maxDistanceMandatory << " --> failure";
#endif
            maxDistanceMandatory = maxDistanceMandatory + 1;
        }

        graph.reset();

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
        qDebug() << "maxDistanceMandatory:" << maxDistanceMandatory;
#endif

        // backup matching
        graph.updateMatching(maxDistanceMandatory);
        auto mates = graph.getMates();

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
        qDebug() << "adding optional open positions";
#endif
        for(auto& node : optionalOpenPositions) {
            graph.addNode(node, true);
        }

        // Now, we try to augment the resulting matching to find the minimal maximum distance such that there is a matching
        // between the particle positions and all open positions that covers at the same time:
        // a) all mandatory positions
        // b) all particle positions
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
        qDebug() << "searching maxDistanceOptional";
#endif
        int maxDistanceOptional = maxPairwiseDistancePowerOfTwo;
        delta = maxDistanceOptional;
        while(delta >= 1) {
            if(maxDistanceOptional < maxDistanceMandatory) {
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
                qDebug() << maxDistanceOptional << " --> below maxDistanceMandatory";
#endif
                maxDistanceOptional = maxDistanceOptional + delta;
                delta = delta / 2;
            } else {
                graph.updateMatching(maxDistanceOptional);
                if(graph.getMatchingSize() == (int) particles.size()) {
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
                    qDebug() << maxDistanceOptional << " --> success";
#endif
                    maxDistanceOptional = maxDistanceOptional - delta;
                    delta = delta / 2;
                } else {
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
                    qDebug() << maxDistanceOptional << " --> failure";
#endif
                    maxDistanceOptional = maxDistanceOptional + delta;
                    delta = delta / 2;
                }
                graph.setMates(mates);
            }
        }

        if(maxDistanceOptional < maxDistanceMandatory) {
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
            qDebug() << maxDistanceOptional << " --> below maxDistanceMandatory";
#endif
            maxDistanceOptional = maxDistanceOptional + 1;
        } else {
            graph.updateMatching(maxDistanceOptional);
            if(graph.getMatchingSize() == (int) particles.size()) {
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
                qDebug() << maxDistanceOptional << " --> success";
#endif
            } else {
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
                qDebug() << maxDistanceOptional << " --> failure";
#endif
                maxDistanceOptional = maxDistanceOptional + 1;
            }
        }

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
        qDebug() << "minimal maximum distance:" << maxDistanceOptional;
        qDebug() << "------------------------------------";
#endif

        return maxDistanceOptional;
    }
}

}

#endif // UNIVERSALCOATINGHELPER

