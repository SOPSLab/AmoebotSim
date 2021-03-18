/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the DynamicDemo code
// tutorial. DynamicDemo demonstrates how to add and remove particles from the
// system at runtime. The pseudocode is available in the docs:
// [https://amoebotsim.rtfd.io/en/latest/tutorials/tutorials.html#dynamicdemo-adding-and-removing-particles].

#ifndef AMOEBOTSIM_ALG_DEMO_DYNAMICDEMO_H_
#define AMOEBOTSIM_ALG_DEMO_DYNAMICDEMO_H_

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class DynamicDemoParticle : public AmoebotParticle {
 public:
  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and growth and death
  // probabilities.
  DynamicDemoParticle(const Node& head, const int globalTailDir,
                      const int orientation, AmoebotSystem& system,
                      const double growProb, const double dieProb);

  // Executes one particle activation.
  void activate() override;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  QString inspectionText() const override;

 protected:
  // Member variables.
  const double _growProb;
  const double _dieProb;

 private:
  friend class DynamicDemoSystem;
};

class DynamicDemoSystem : public AmoebotSystem {
 public:
  // Constructs a system of DynamicDemoParticles with an optionally specified
  // size (#particles) and particle growth and death probabilities.
  DynamicDemoSystem(unsigned int numParticles = 10, double growProb = 0.02,
                    double dieProb = 0.01);

  // Returns true when the simulation has completed; i.e, when all particles
  // have died.
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_DEMO_DYNAMICDEMO_H_
