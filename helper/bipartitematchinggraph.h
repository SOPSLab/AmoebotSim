#ifndef BIPARTITEMATCHINGGRAPH
#define BIPARTITEMATCHINGGRAPH

#include <QDebug>

#include <deque>
#include <set>

#include "sim/node.h"
#include "misc.h"
#include "rect.h"

class BipartiteMatchingGraph
{
private:
    struct GraphNode
    {
        GraphNode(int id, Node node, bool right) : id(id), node(node), right(right), mate(nullptr), pred(nullptr) { }

        int id;
        Node node;
        bool right;
        GraphNode* mate;
        GraphNode* pred;
        std::deque<int> distances;
    };

public:
    BipartiteMatchingGraph(const std::set<Node>& object) : object(object) { }

    ~BipartiteMatchingGraph()
    {
        for(GraphNode* node : nodes) {
            delete node;
        }
    }

    void addNode(Node node, bool right)
    {
        GraphNode* graphNode = new GraphNode(nodes.size(), node, right);
        nodes.push_back(graphNode);

        // setup distances
        for(GraphNode* node : nodes) {
            node->distances.resize(nodes.size());
        }

        std::deque<GraphNode*> undiscoveredGraphNodes = nodes;

        std::set<Node> visitedNodes;
        visitedNodes.insert(graphNode->node);

        Rect boundingRect = getLooseBoundingRect();

        int distance = 0;
        std::set<Node> lastLayer;
        while(!undiscoveredGraphNodes.empty()) {
            // check whether a graph node was discovered and if so set the distance accordingly
            for(int i = 0; i < (int) undiscoveredGraphNodes.size(); i++) {
                GraphNode* otherGraphNode = undiscoveredGraphNodes.at(i);
                if(visitedNodes.find(otherGraphNode->node) != visitedNodes.end()) {
                    graphNode->distances[otherGraphNode->id] = distance;
                    otherGraphNode->distances[graphNode->id] = distance;
                    std::swap(undiscoveredGraphNodes[i], undiscoveredGraphNodes[0]);
                    undiscoveredGraphNodes.pop_front();
                    i--; // adjust for removed element
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

    void updateMatching(const int distanceLimit)
    {
        // augment as far as possible
        while(augment(distanceLimit)) {
            Q_ASSERT(isValid());
        }
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
            for(int distance : node->distances) {
                if(distance > max) {
                    max = distance;
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

    void printMatching()
    {
        for(GraphNode* node : nodes) {
            if(!node->right && node->mate != nullptr) {
                qDebug() << "(" << node->node.x << ", " << node->node.y << ") <--> (" << node->mate->node.x << ", " << node->mate->node.y << ")";
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

            if(node->distances.size() != nodes.size()) {
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
        for(const Node& node : object) {
            rect.include(node);
        }

        rect.minX = rect.minX - 1;
        rect.maxX = rect.maxX + 1;
        rect.minY = rect.minY - 1;
        rect.maxY = rect.maxY + 1;

        return rect;
    }

    bool areNeighbors(const int distanceLimit, GraphNode* node1, GraphNode* node2)
    {
        return (node1->right != node2->right) && (node1->distances.at(node2->id) <= distanceLimit);
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
        for(int i = 0; i < (int) unvisitedNodes.size(); i++) {
            GraphNode* node = unvisitedNodes.at(i);
            if(!node->right && node->mate == nullptr) {
                node->pred = node;
                queue.push_back(node);
                visitedNodes.push_back(node);
                std::swap(unvisitedNodes[i], unvisitedNodes[0]);
                unvisitedNodes.pop_front();
                i--; // adjust for removed element
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
                    i--; // adjust for removed element

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


#endif // BIPARTITEMATCHINGGRAPH

