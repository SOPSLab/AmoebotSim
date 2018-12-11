// Defines a particle that complies with the properties and constraints of the
// amoebot model, but is independent of any particular algorithm.

#ifndef AMOEBOTSIM_ALG_AMOEBOTPARTICLE_H
#define AMOEBOTSIM_ALG_AMOEBOTPARTICLE_H

#include <array>
#include <deque>
#include <functional>
#include <map>
#include <memory>

#include "core/amoebotsystem.h"
#include "core/localparticle.h"
#include "helper/randomnumbergenerator.h"
#include "core/node.h"

class AmoebotParticle : public LocalParticle, public RandomNumberGenerator {
 public:
  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, and a particle system to belong to.
  AmoebotParticle(const Node& head, int globalTailDir, const int orientation,
                  AmoebotSystem& system);

  // Deletes the tokens this particle holds before destructing the particle.
  // These deletions are handled by the shared_ptrs.
  virtual ~AmoebotParticle();

  // Executes one particle activation. The '= 0' indicates that this is a pure
  // virtual function which must be overridden by any particle subclasses.
  virtual void activate() = 0;

  // Returns the global direction from the head (respectively, tail) on which to
  // draw the direction markers (-1 indicates no marker). Meant to provide info
  // to the visualization and should not be called by any particle algorithms.
  int headMarkGlobalDir() const final;
  int tailMarkGlobalDir() const final;

 protected:
  // A struct expressing the most basic version of a token. Particle subclasses
  // using tokens should write their token structs to inherit from this one.
  struct Token { virtual ~Token(){ } };

  // Returns the local directions from the head (respectively, tail) on which to
  // draw the direction markers. Intended to be overridden by particle
  // subclasses, as the default implementations return -1 (no markers).
  virtual int headMarkDir() const;
  virtual int tailMarkDir() const;

  // Functions for expansion. canExpand checks if this particle can expand in
  // the direction of the specified port by seeing if it is currently contracted
  // and the desired position is unoccupied. expand performs the expansion.
  bool canExpand(int label) const;
  void expand(int label);

  // Functions for handover expansion. canPush checks if this particle is
  // contracted and the position in the direction of the specified port is
  // occupied by a neighboring expanded particle. push performs the handover.
  bool canPush(int label) const;
  void push(int label);

  // Functions for contraction. contractHead and contractTail are self-
  // explanatory, while contract contracts either the particle's head or tail
  // depending on the port label provided.
  void contract(int label);
  void contractHead();
  void contractTail();

  // Functions for handover contraction. canPull checks if this particle is
  // expanded and the position in the direction of the specified port is
  // occupied by a neighboring contracted particle. pull performs the handover.
  bool canPull(int label) const;
  void pull(int label);

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  template<class ParticleType>
  ParticleType& nbrAtLabel(int label) const;

  // Functions for checking the existence of a neighboring particle (or more
  // specifically, a neighboring particle's head or tail) in the position
  // incident to the given port.
  bool hasNbrAtLabel(int label) const;
  bool hasHeadAtLabel(int label);
  bool hasTailAtLabel(int label);

  // Function for checking the existence of a neighboring object
  bool hasObjectAtLabel(int label) const;

  // Function for returning the label of the first port incident to a
  // neighboring object, starting at the (optionally) specified label and
  // continuing counter-clockwise
  int labelOfFirstObjectNbr(int startLabel = 0) const;

  // Returns the label of the first port incident to a neighboring particle
  // that satisfies the specified property, starting at the (optionally)
  // specified label and continuing counter-clockwise.
  template<class ParticleType>
  int labelOfFirstNbrWithProperty(
      std::function<bool(const ParticleType&)> propertyCheck,
      int startLabel = 0) const;

  // Functions for handling tokens. putToken simply adds the given token pointer
  // to this particle's collection of tokens. peekAtToken returns a pointer to
  // the first token in this particle's collection which is of the specified
  // type. takeToken does the same thing as peekAtToken but additionally removes
  // the returned token pointer from this particle's collection. Note that
  // peekAtToken and takeToken both fail when no token of the given type exists
  // in the collection; consider using hasToken() first if unsure.
  void putToken(std::shared_ptr<Token> token);
  template<class TokenType>
  std::shared_ptr<TokenType> peekAtToken() const;
  template<class TokenType>
  std::shared_ptr<TokenType> takeToken();

  // Functions for basic token-related information. countTokens returns the
  // number of tokens of the specified type this particle has in its collection.
  // hasToken checks whether this particle has at least one token of the given
  // type; i.e., equivalent to countTokens > 0.
  template<class TokenType>
  int countTokens() const;
  template<class TokenType>
  bool hasToken() const;

  AmoebotSystem& system;
 private:
  std::deque<std::shared_ptr<Token>> tokens;
};

template<class ParticleType>
ParticleType& AmoebotParticle::nbrAtLabel(int label) const {
  Node nbrNode = nbrNodeReachedViaLabel(label);
  auto it = system.particleMap.find(nbrNode);
  Q_ASSERT(it != system.particleMap.end() &&
           dynamic_cast<ParticleType*>(it->second) != nullptr);

  return dynamic_cast<ParticleType&>(*(it->second));
}

template<class ParticleType>
int AmoebotParticle::labelOfFirstNbrWithProperty(
    std::function<bool(const ParticleType&)> propertyCheck,
    int startLabel) const {
  const int labelLimit = isContracted() ? 6 : 10;
  for (int labelOffset = 0; labelOffset < labelLimit; labelOffset++) {
    const int label = (startLabel + labelOffset) % labelLimit;
    if (hasNbrAtLabel(label)) {
      const ParticleType& particle = nbrAtLabel<ParticleType>(label);
      if (propertyCheck(particle)) {
        return label;
      }
    }
  }

  return -1;
}

template<class TokenType>
std::shared_ptr<TokenType> AmoebotParticle::peekAtToken() const {
  for (unsigned int i = 0; i < tokens.size(); i++) {
    std::shared_ptr<TokenType> token =
        std::dynamic_pointer_cast<TokenType>(tokens[i]);
    if (token != nullptr) {
      return token;
    }
  }
  Q_ASSERT(false);
}

template<class TokenType>
std::shared_ptr<TokenType> AmoebotParticle::takeToken() {
  for (unsigned int i = 0; i < tokens.size(); i++) {
    std::shared_ptr<TokenType> token =
        std::dynamic_pointer_cast<TokenType>(tokens[i]);
    if (token != nullptr) {
      std::swap(tokens[0], tokens[i]);
      tokens.pop_front();
      return token;
    }
  }
  Q_ASSERT(false);
}

template<class TokenType>
int AmoebotParticle::countTokens() const {
  int count = 0;
  for (unsigned int i = 0; i < tokens.size(); i++) {
    std::shared_ptr<TokenType> token =
        std::dynamic_pointer_cast<TokenType>(tokens[i]);
    if (token != nullptr) {
      count++;
    }
  }
  return count;
}

template<class TokenType>
bool AmoebotParticle::hasToken() const {
  for (unsigned int i = 0; i < tokens.size(); i++) {
    std::shared_ptr<TokenType> token =
        std::dynamic_pointer_cast<TokenType>(tokens[i]);
    if (token != nullptr) {
      return true;
    }
  }
  return false;
}

#endif  // AMOEBOTSIM_ALG_AMOEBOTPARTICLE_H
