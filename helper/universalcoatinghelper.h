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

inline int getLowerBoundbla(const System& system)
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

        for(GraphNode* otherGraphNode : nodes) {
            int dist = distance(graphNode, otherGraphNode);
            otherGraphNode->distance.insert(std::pair<GraphNode*, int>(graphNode, dist));
            graphNode->distance.insert(std::pair<GraphNode*, int>(otherGraphNode, dist));
        }
    }

    void updateMatching(const int weightLimit)
    {
        // augment as far as possible
        while(augment(weightLimit)) {
            Q_ASSERT(isValid());
        }

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
        report();
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

private:
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
    void report()
    {
        int matchingSize = 0;
        for(GraphNode* node : nodes) {
            if(!node->right && node->mate != nullptr) {
                matchingSize++;
#ifdef UNIVERSAL_COATING_HELPER_DEBUG_MATCHING
                qDebug() << "(" << node->node.x << ", " << node->node.y << ") <--> (" << node->mate->node.x << ", " << node->mate->node.y << ")";
#endif
            }
        }

        qDebug() << "matching size:" << matchingSize;
    }
#endif

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

    int distance(GraphNode* graphNode1, GraphNode* graphNode2)
    {
        // use bfs to determine the length of a shortest path between the nodes that does not intersect the object
        Node node1 = graphNode1->node;
        Node node2 = graphNode2->node;

        Q_ASSERT(object.find(node1) == object.end() && object.find(node2) == object.end());

        std::set<Node> visitedNodes;
        visitedNodes.insert(node1);
        int distance = 0;
        while(visitedNodes.find(node2) == visitedNodes.end()) {
            std::set<Node> layer;
            for(const Node& node : visitedNodes) {
                for(int i = 0; i < 6; i++) {
                    Node neighbor = node.nodeInDir(i);
                    if(visitedNodes.find(neighbor) == visitedNodes.end() && object.find(neighbor) == object.end()) {
                        layer.insert(neighbor);
                    }
                }
            }

            visitedNodes.insert(layer.cbegin(), layer.cend());
            distance++;
        }
        return distance;
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

inline int getLowerBound(const System& system)
{
#ifdef UNIVERSAL_COATING_HELPER_DEBUG
    qDebug() << "------------------------------------";
#endif

    std::set<Node> object, particles;
    setupObjectAndParticles(system, object, particles);

    std::set<Node> mandatoryOpenPositions, optionalOpenPositions;
    setupOpenPositions(object, particles, mandatoryOpenPositions, optionalOpenPositions);

    Graph graph(object);

    qDebug() << "add particles";

    for(auto& node : particles) {
        graph.addNode(node, false);
    }

    qDebug() << "add open positions";

    for(auto& node : mandatoryOpenPositions) {
        graph.addNode(node, true);
    }

    qDebug() << "calculate matchings";

    for(int i = 0; i < 20; i++) {
        qDebug() << "i:" << i;
        graph.updateMatching(i);
        qDebug() << "matchingSize" << graph.getMatchingSize();
        graph.reset();
    }

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
    qDebug() << "------------------------------------";
#endif

    return -1;
}

}

#endif // UNIVERSALCOATINGHELPER

