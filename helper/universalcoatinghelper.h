#ifndef UNIVERSALCOATINGHELPER
#define UNIVERSALCOATINGHELPER

#include <QDebug>

#include <deque>
#include <functional>
#include <set>

#include "sim/node.h"
#include "sim/particle.h"
#include "bipartitematchinggraph.h"
#include "misc.h"
#include "rect.h"

// use this to get some debug output
//#define UNIVERSAL_COATING_HELPER_DEBUG

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

inline int setupOpenPositions(  const std::set<Node>& object,
                                const std::set<Node>& particles,
                                std::set<Node>& mandatoryOpenPositions,
                                std::set<Node>& optionalOpenPositions)
{
    int numMandatoryLayers = 0;
    std::set<Node> growingObject;
    growingObject.insert(object.cbegin(), object.cend());

    std::set<Node> layer = growingObject;
    while(true) {
        layer = nextLayer(layer, [&](const Node& node) { return growingObject.find(node) == growingObject.end(); });

        if(mandatoryOpenPositions.size() + layer.size() <= particles.size()) {
            // found a layer of mandatory positions.
            mandatoryOpenPositions.insert(layer.cbegin(), layer.cend());
            growingObject.insert(layer.cbegin(), layer.cend());
            numMandatoryLayers++;
        } else {
            // the first layer that cannot be added to mandatoryOpenPositions is the set of optional open positions
            optionalOpenPositions = layer;
            break; // done
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

    for(const Node& node : particles) {
        mandatoryOpenPositions.erase(node);
        optionalOpenPositions.erase(node);
    }

    std::set<Node> growingParticles;
    growingParticles.insert(particles.cbegin(), particles.cend());

    std::set<Node> layer = growingParticles;
    while(!mandatoryOpenPositions.empty() || (int) optionalOpenPositions.size() > numOptionalOpenPositionsToRemain) {
        // add another layer to the growingParticles
        layer = nextLayer(layer, [&](const Node& node) {
           return growingParticles.find(node) == growingParticles.end() && object.find(node) == object.end();
        });

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

inline int getStrongLowerBound(const System& system)
{
    std::set<Node> object, particles;
    setupObjectAndParticles(system, object, particles);

    std::set<Node> mandatoryOpenPositions, optionalOpenPositions;
    int numMandatoryLayers = setupOpenPositions(object, particles, mandatoryOpenPositions, optionalOpenPositions);

    // setup graph
    BipartiteMatchingGraph graph(object);
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

