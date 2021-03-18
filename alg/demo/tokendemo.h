/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines a particle system and composing particles for the TokenDemo code
// tutorial. TokenDemo demonstrates token passing functionality, including
// defining new token types, modifying token memory contents, and passing tokens
// between particles. The description and tutorial is available in the docs:
// [https://amoebotsim.rtfd.io/en/latest/tutorials/tutorials.html#tokendemo-communicating-over-distance].

#ifndef AMOEBOTSIM_ALG_DEMO_TOKENDEMO_H_
#define AMOEBOTSIM_ALG_DEMO_TOKENDEMO_H_

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class TokenDemoParticle : public AmoebotParticle {
 public:
  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, and a system which it belongs to.
  TokenDemoParticle(const Node& head, const int globalTailDir,
                    const int orientation, AmoebotSystem& system);

  // Executes one particle activation.
  void activate() override;

  // Returns the color to be used for the ring drawn around the head node. In
  // this case, it returns the color of the token(s) this particle is holding.
  int headMarkColor() const override;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  QString inspectionText() const override;

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  TokenDemoParticle& nbrAtLabel(int label) const;

 protected:
  // Token types. DemoToken is a general type that has two data members:
  // (i) _passedFrom, which denotes the direction from which the token was last
  // passed (initially -1, meaning it has not yet been passed), and (ii)
  // _lifetime, which is decremented each time the token is passed. The red and
  // blue tokens are two types of DemoTokens.
  struct DemoToken : public Token { int _passedFrom = -1; int _lifetime; };
  struct RedToken : public DemoToken {};
  struct BlueToken : public DemoToken {};

 private:
  friend class TokenDemoSystem;
};

class TokenDemoSystem : public AmoebotSystem {
 public:
  // Constructs a system of TokenDemoParticles with an optionally specified size
  // (#particles) and token lifetime.
  TokenDemoSystem(int numParticles = 48, int lifetime = 100);

  // Returns true when the simulation has completed; i.e, when all tokens have
  // died out.
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_DEMO_TOKENDEMO_H_
