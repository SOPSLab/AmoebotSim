// Defines the particle system and composing particles for the swarm aggregation
// algorithm as defined in 'Self-organized aggregation without computation'
// [journals.sagepub.com/doi/abs/10.1177/0278364914525244].
//
// Run with aggregation(#particles) on the simulator command line.
//
// Basic description:
// The initial state of the system is created by randomly distributing particles
// within a box. After this, the particles attempt to aggregate together by
// executing the following rules:
// 1) If there is a particle in sight, rotate in place, thus shifting the
// particle's center of rotation.
// 2) If there is no such particle in sight, move around the center of rotation,
// thus shifting the particle's position as well as the direction of its field
// of vision.

#ifndef AMOEBOTSIM_ALG_AGGREGATION_H
#define AMOEBOTSIM_ALG_AGGREGATION_H

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class AggregateParticle : public AmoebotParticle {
  friend class MaxDistanceMeasure;
  friend class SumDistancesMeasure;

 public:
  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, the direction of the
  // center of rotation for the particle, and a list containing all of the
  // particles currently within the system.
  AggregateParticle(const Node head, const int globalTailDir,
               const int orientation, AmoebotSystem& system, int center,
               std::vector<AggregateParticle*> particles);

  // Executes one particle activation.
  virtual void activate();

  // Functions for altering a particle's cosmetic appearance; headMarkColor
  // (respectively, tailMarkColor) returns the color to be used for the ring
  // drawn around the head (respectively, tail) node. Tail color is not shown
  // when the particle is contracted. headMarkDir returns the label of the port
  // on which the black head marker is drawn.
  virtual int headMarkColor() const;
  virtual int headMarkDir() const;
  virtual int tailMarkColor() const;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  virtual QString inspectionText() const;

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  AggregateParticle& nbrAtLabel(int label) const;

  // Helper function to determine whether or not a particle may be seen within
  // the current particle's field of vision. The field of vision that the
  // particle uses is determined by the cone starting from (center + 5) % 6 and
  // ending at (center + 4) % 6
  bool checkIfParticleInSight() const;

 protected:
  int center;
  int perturb = 0;
  std::vector<AggregateParticle*> particles;

 private:
  friend class AggregateSystem;
};

class AggregateSystem : public AmoebotSystem  {
  friend class MaxDistanceMeasure;
  friend class SumDistancesMeasure;

 public:
  // Constructs a system of AggregateParticles with an optionally specified size
  // (#particles). The spread of the particles in the system is limited by a box
  // whose dimensions are based off of the number of particles available in the
  // system (with a lower bound of 50 for the box dimensions)
  AggregateSystem(int numParticles = 2);

  // Checks whether or not the system's run of the aggregation algorithm has
  // terminated (all particles have clustered together).
  bool hasTerminated() const override;
};

class MaxDistanceMeasure : public Measure {

public:
  MaxDistanceMeasure(const QString name, const unsigned int freq,
                       AggregateSystem& system);

  double calculate() const final;

protected:
  AggregateSystem& _system;
};

class SumDistancesMeasure : public Measure {

public:
  SumDistancesMeasure(const QString name, const unsigned int freq,
                       AggregateSystem& system);

  double calculate() const final;

protected:
  AggregateSystem& _system;
};

#endif  // AMOEBOTSIM_ALG_AGGREGATION_H
