#ifndef AMOEBOTPARTICLE_H
#define AMOEBOTPARTICLE_H

#include <array>
#include <map>

#include "alg/labellednocompassparticle.h"
#include "sim/node.h"

class AmoebotParticle : public LabelledNoCompassParticle
{
public:
    AmoebotParticle(const Node head,
                    const int globalTailDir,
                    const int orientation,
                    std::map<Node, AmoebotParticle*>& particleMap);

    virtual void activate() = 0;

protected:
    bool canExpand(int label);
    bool expand(int label);

    void contractHead();
    void contractTail();
    void contract(int label);

    bool hasNeighborAtLabel(int label);

    template<class ParticleType>
    ParticleType& neighborAtLabel(int label);

private:
    std::map<Node, AmoebotParticle*>& particleMap;
};


template<class ParticleType>
ParticleType& AmoebotParticle::neighborAtLabel(int label)
{
    Node node = neighboringNodeReachedViaLabel(label);
    auto it = particleMap.find(node);
    if(it == particleMap.end()) {
        Q_ASSERT(false);
    }

    return dynamic_cast<ParticleType&>(*((*it).second));
}
#endif // AMOEBOTPARTICLE_H
