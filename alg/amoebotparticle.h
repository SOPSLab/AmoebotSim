#ifndef AMOEBOTPARTICLE_H
#define AMOEBOTPARTICLE_H

#include <array>
#include <deque>
#include <functional>
#include <map>

#include "alg/labellednocompassparticle.h"
#include "helper/randomnumbergenerator.h"
#include "sim/node.h"

class AmoebotParticle : public LabelledNoCompassParticle, public RandomNumberGenerator
{
protected:
    struct Token { virtual ~Token(){ } };

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

    bool hasNeighborAtLabel(int label) const;
    virtual AmoebotParticle& neighborAtLabel(int label) const;
    bool hasHeadAtLabel(int label);
    bool hasTailAtLabel(int label);

    template<class ParticleType>
    int labelOfFirstNeighborWithProperty(std::function<bool(const ParticleType&)> propertyCheck, int startLabel = 0) const;

    void putToken(Token* token);
    template<class TokenType>
    TokenType* takeToken();
    template<class TokenType>
    int countTokens() const;
    template<class TokenType>
    bool hasToken() const;

private:
    std::map<Node, AmoebotParticle*>& particleMap;
    std::deque<Token*> tokens;
};

template<class ParticleType>
int AmoebotParticle::labelOfFirstNeighborWithProperty(std::function<bool(const ParticleType&)> propertyCheck, int startLabel) const
{
    int labelLimit = isContracted() ? 6 : 10;
    for(int labelOffset = 0; labelOffset < labelLimit; labelOffset++) {
        int label = (startLabel + labelOffset) % labelLimit;
        if(hasNeighborAtLabel(label)) {
            const ParticleType& particle = dynamic_cast<const ParticleType&>(neighborAtLabel(label));
            if(propertyCheck(particle)) {
                return label;
            }
        }
    }
    return -1;
}

template<class TokenType>
TokenType* AmoebotParticle::takeToken()
{
    for(unsigned int i = 0; i < tokens.size(); i++) {
        TokenType* token = dynamic_cast<TokenType*>(tokens[i]);
        if(token != nullptr) {
            std::swap(tokens[0], tokens[i]);
            tokens.pop_front();
            return token;
        }
    }
    Q_ASSERT(false);
}

template<class TokenType>
int AmoebotParticle::countTokens() const
{
    int count = 0;
    for(unsigned int i = 0; i < tokens.size(); i++) {
        TokenType* token = dynamic_cast<TokenType*>(tokens[i]);
        if(token != nullptr) {
            count++;
        }
    }
    return count;
}

template<class TokenType>
bool AmoebotParticle::hasToken() const
{
    for(unsigned int i = 0; i < tokens.size(); i++) {
        TokenType* token = dynamic_cast<TokenType*>(tokens[i]);
        if(token != nullptr) {
            return true;
        }
    }
    return false;
}

#endif // AMOEBOTPARTICLE_H
