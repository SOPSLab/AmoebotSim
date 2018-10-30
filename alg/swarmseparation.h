// Defines the particle system and composing particles for Roderich Gross's
// swarm separation algorithm described in 'Segregation in Swarms of Mobile
// Robots Based on the Brazil Nut Effect' [doi.org/10.1109/IROS.2009.5353942]
// and 'Segregation in Swarms of e-puck Robots Based On the Brazil Nut Effect'
// [https://dl.acm.org/citation.cfm?id=2343599].
//
// Run with swarmseparation(#particles, randConstant) on the simulator command
// line.

#ifndef AMOEBOTSIM_ALG_SWARMSEPARATION_H
#define AMOEBOTSIM_ALG_SWARMSEPARATION_H

#include <QString>
#include <utility>

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class SwarmSeparationParticle : public AmoebotParticle {
 public:
  enum class State {
    Small,
    Medium,
    Large
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, an initial state, and
  // a constant parameterizing the strength of its random motion.
  SwarmSeparationParticle(const Node head, const int globalTailDir,
                          const int orientation, AmoebotSystem& system,
                          State state, double c_rand, double c_repulse);

  // Executes one particle activation.
  virtual void activate();

  // Functions for altering a particle's cosmetic appearance; headMarkColor
  // (respectively, tailMarkColor) returns the color to be used for the ring
  // drawn around the head (respectively, tail) node. Tail color is not shown
  // when the particle is contracted.
  virtual int headMarkColor() const;
  virtual int tailMarkColor() const;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  virtual QString inspectionText() const;

  // Calculates the sum of the three movement vectors: taxis (towards the light
  // at (0,0)), random (a unit vector in (0, 2pi)), and repulsion (weighted sum
  // of "collisions" with other particles). Returns the discretized movement
  // direction closest to the resulting vector.
  int vectorCalculation();

 protected:
  State state;
  double c_rand;
  double c_repulse;

 private:
  friend class SwarmSeparationSystem;

  // Each get vector function returns a unit vector pointing towards the
  // respective goal direction. Taxis returns a unit vector pointing towards the
  // light source at (0,0), Rand returns a unit vector pointing in a random
  // direction in [0,2pi), and Repulse gets a unit vector pointing away from the
  // summed "collisions" with other particles. The final getMoveVector() returns
  // the weighted sum of the previous three according to the constants c_rand
  // and c_repulse.
  std::pair<double, double> getTaxisVector();
  std::pair<double, double> getRandVector();
  std::pair<double, double> getRepulseVector();
  std::pair<double, double> getMoveVector();

  // Utility functions for vectors: the first computes the magnitude of the
  // vector, and the second normalizes the vector in place.
  double magnitude(const std::pair<double, double> &vec);
  void normalize(std::pair<double, double> &vec);

  // Utility functions for transforming coordinates in the triangular lattice to
  // Cartesian coordinates.
  double xTriToCart(const double x, const double y);
  double yTriToCart(const double y);

  // Utility function for getting the Euclidean distance between two nodes. Note
  // that Nodes as defined in this simulator are in the triangular lattice, so
  // this function first transforms them into Cartesian coordinates before
  // computing distance.
  double nodeDist(const Node &n1, const Node &n2);
};

class SwarmSeparationSystem : public AmoebotSystem {
 public:
  // Constructs a system of SwarmSeparationParticles with an optionally
  // specified size (#particles), and random constant (which is at least 0).
  SwarmSeparationSystem(int numParticles = 200, double c_rand = 0.7,
                        double c_repulse = 0.5);

  // Because this algorithm never terminates, this simply returns false.
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_SWARMSEPARATION_H
