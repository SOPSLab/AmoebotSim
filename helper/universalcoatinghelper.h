#ifndef UNIVERSALCOATINGHELPER
#define UNIVERSALCOATINGHELPER

#include<QDebug>
#include <set>

#include "sim/node.h"
#include "sim/particle.h"

// use this to get some debug output
#define UNIVERSAL_COATING_HELPER_DEBUG

namespace UniversalCoating
{
int getLowerBound(const System& system);

inline int getLowerBound(const System& system)
{
    std::set<Node> object, particles;

    // setup object and particles
    for(int i = 0; i < system.getNumParticles(); i++) {
        const Particle& p = system.at(i);
        if(p.tailDir != -1) {
            // this should be an initial configuration
            // do not allow expanded particles
            return -1;
        }
        if(p.isStatic()) {
            object.insert(p.head);
        } else {
            particles.insert(p.head);
        }
    }

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
    qDebug() << "------------------------------------";
    qDebug() << "numObjectPositions:" << object.size();
    qDebug() << "numParticlePositions:" << particles.size();
#endif

    std::set<Node> mandatoryOpenPositions, optionalOpenPositions;
    int numMandatoryLayers = 0;

    {   // setup mandatoryOpenPositions and optionalOpenPositions
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
    }

#ifdef UNIVERSAL_COATING_HELPER_DEBUG
    qDebug() << "numMandatoryLayers:" << numMandatoryLayers;
    qDebug() << "numMandatoryOpenPositions:" << mandatoryOpenPositions.size();
    qDebug() << "numOptionalOpenPositions:" << optionalOpenPositions.size();
#endif

    Q_ASSERT(mandatoryOpenPositions.size() <= particles.size());
    Q_ASSERT(mandatoryOpenPositions.size() + optionalOpenPositions.size() >= particles.size());

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

}

#endif // UNIVERSALCOATINGHELPER

