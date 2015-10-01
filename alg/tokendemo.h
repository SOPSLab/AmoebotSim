#ifndef TOKENDEMO_H
#define TOKENDEMO_H

#include "alg/hexagon.h"

class TokenDemoParticle : public HexagonParticle
{
protected:
    struct RedToken : public Token { };
    struct BlueToken : public Token { };

public:
    TokenDemoParticle(const Node head,
                      const int globalTailDir,
                      const int orientation,
                      std::map<Node, AmoebotParticle*>& particleMap,
                      State state);

    virtual void activate();

    virtual int headMarkColor() const;

    virtual QString inspectionText() const;

    TokenDemoParticle& neighborAtLabel(int label) const;
};

class TokenDemoSystem : public AmoebotSystem
{
public:
    TokenDemoSystem(int numParticles = 20, float holeProb = 0.2);

    virtual bool hasTerminated() const;
};

#endif // TOKENDEMO_H
