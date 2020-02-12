/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines a particle for demonstrating token passing functionality.
//
// Run with tokendemo(#particles, hole probability) on the simulator command
// line.

#ifndef AMOEBOTSIM_ALG_DEMO_TOKENDEMO_H_
#define AMOEBOTSIM_ALG_DEMO_TOKENDEMO_H_

#include <QString>

#include "alg/shapeformation.h"
#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class TokenDemoParticle : public ShapeFormationParticle {
 public:
  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, an initial state, and
  // a string to determine its shape formation mode ("l" for line formation in
  // this demo).
  TokenDemoParticle(const Node head, const int globalTailDir,
                    const int orientation, AmoebotSystem& system, State state,
                    const QString mode);

  // Executes one particle activation.
  virtual void activate();

  // Returns the color to be used for the ring drawn around the head node. In
  // this case, it returns the color of the token(s) this particle is holding.
  virtual int headMarkColor() const;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  virtual QString inspectionText() const;

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  TokenDemoParticle& nbrAtLabel(int label) const;

 protected:
  // Tokens for demonstration. In practice, these tokens can contain a constant
  // amount of structured data; however, for demonstration these are empty.
  struct RedToken : public Token {};
  struct BlueToken : public Token {};
};

class TokenDemoSystem : public AmoebotSystem {
 public:
  // Constructs a system of TokenDemoParticles with an optionally specified size
  // (#particles) and hole probability. holeProb in [0,1] controls how "spread
  // out" the system is; closer to 0 is more compressed, closer to 1 is more
  // expanded.
  TokenDemoSystem(int numParticles = 20, double holeProb = 0.2);

  // Returns true when the simulation has completed; however, as this demo runs
  // indefinitely, it always returns false.
  virtual bool hasTerminated() const;
};

#endif  // AMOEBOTSIM_ALG_DEMO_TOKENDEMO_H_
