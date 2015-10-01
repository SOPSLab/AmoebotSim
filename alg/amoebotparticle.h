#ifndef AMOEBOTPARTICLE_H
#define AMOEBOTPARTICLE_H

#include <array>
#include <deque>
#include <functional>
#include <map>
#include <memory>

#include "alg/amoebotsystem.h"
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
                    AmoebotSystem& system);

    virtual void activate() = 0;

    virtual int headMarkGlobalDir() const final;
    virtual int tailMarkGlobalDir() const final;

protected:
    virtual int headMarkDir() const;
    virtual int tailMarkDir() const;

    bool canExpand(int label);
    void expand(int label);

    bool canPush(int label);
    void push(int label);

    void contract(int label);
    void contractHead();
    void contractTail();

    bool canPull(int label);
    void pull(int label);

    template<class ParticleType>
    ParticleType& neighborAtLabel(int label) const;

    bool hasNeighborAtLabel(int label) const;
    bool hasHeadAtLabel(int label);
    bool hasTailAtLabel(int label);

    template<class ParticleType>
    int labelOfFirstNeighborWithProperty(std::function<bool(const ParticleType&)> propertyCheck, int startLabel = 0) const;

    void putToken(std::shared_ptr<Token> token);
    template<class TokenType>
    std::shared_ptr<TokenType> takeToken();
    template<class TokenType>
    int countTokens() const;
    template<class TokenType>
    bool hasToken() const;

private:
    AmoebotSystem& system;
    std::deque<std::shared_ptr<Token>> tokens;
};

template<class ParticleType>
ParticleType& AmoebotParticle::neighborAtLabel(int label) const
{
    Node neighboringNode = neighboringNodeReachedViaLabel(label);
    auto it = system.particleMap.find(neighboringNode);
    Q_ASSERT(it != system.particleMap.end() && dynamic_cast<ParticleType*>(it->second) != nullptr);
    return dynamic_cast<ParticleType&>(*(it->second));
}

template<class ParticleType>
int AmoebotParticle::labelOfFirstNeighborWithProperty(std::function<bool(const ParticleType&)> propertyCheck, int startLabel) const
{
    const int labelLimit = isContracted() ? 6 : 10;
    for(int labelOffset = 0; labelOffset < labelLimit; labelOffset++) {
        const int label = (startLabel + labelOffset) % labelLimit;
        if(hasNeighborAtLabel(label)) {
            const ParticleType& particle = neighborAtLabel<ParticleType>(label);
            if(propertyCheck(particle)) {
                return label;
            }
        }
    }
    return -1;
}

template<class TokenType>
std::shared_ptr<TokenType> AmoebotParticle::takeToken()
{
    for(unsigned int i = 0; i < tokens.size(); i++) {
        std::shared_ptr<TokenType> token = std::dynamic_pointer_cast<TokenType>(tokens[i]);
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
        std::shared_ptr<TokenType> token = std::dynamic_pointer_cast<TokenType>(tokens[i]);
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
        std::shared_ptr<TokenType> token = std::dynamic_pointer_cast<TokenType>(tokens[i]);
        if(token != nullptr) {
            return true;
        }
    }
    return false;
}

#endif // AMOEBOTPARTICLE_H
