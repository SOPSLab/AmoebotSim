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
    virtual AmoebotParticle& neighborAtLabel(int label);

private:
    std::map<Node, AmoebotParticle*>& particleMap;
};

#endif // AMOEBOTPARTICLE_H
