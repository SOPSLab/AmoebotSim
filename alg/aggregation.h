// Defines the particle system and composing particles for the swarm aggregation
// algorithm as defined in 'Self-organized aggregation without computation'
// [journals.sagepub.com/doi/abs/10.1177/0278364914525244]. This implementation
// varies from the above Gauci et. al algorithm due to the inclusion of noise.
//
// Run with aggregation(#particles, mode, noiseVal) on the
// simulator command line.
//
// Basic description:
// The initial state of the system is created by randomly distributing particles
// within a box. After this, the particles attempt to aggregate together by
// executing the following rules:
// 1) If there is a particle in sight, rotate in place clockwise, thus shifting
// the particle's center of rotation.
// 2) If there is no such particle in sight, move around the center of rotation
// in a clockwise direction, thus shifting the particle's position as well as
// the direction of its field of vision.
//
// Two forms of noise:
// 1) Deadlock Perturbation (mode = "d") - If blocked when attempting the move
// around the center of rotation (deadlock state), rotate in place to break
// symmetry. noiseVal (integer > 0) represents the number of consecutive
// deadlock state activations needed before perturbing.
// 2) Error Probability (mode = "e") - checkIfParticleSight() function outputs
// the incorrect reading at a certain probability p. noiseVal (double in [0,1])
// represents the error probability p.
// Note: to achieve an algorithm with zero noise, use mode="e" and noiseVal=0

#ifndef AMOEBOTSIM_ALG_AGGREGATION_H
#define AMOEBOTSIM_ALG_AGGREGATION_H

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class AggregateParticle : public AmoebotParticle {
  friend class ConvexHullMeasure;
  friend class SEDMeasure;
  friend class DispersionMeasure;
  friend class ClusterFractionMeasure;

 public:
  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, the direction of the
  // center of rotation for the particle, the form of noise being used, the
  // amount/value of the noise, and a list containing all of the particles
  // currently within the system.
  AggregateParticle(const Node head, const int globalTailDir, const int
                    orientation, AmoebotSystem& system, int center,
                    const QString mode, const double noiseVal,
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
  QString mode;
  double noiseVal;
  int perturb;
  bool visited;
  std::vector<AggregateParticle*> particles;

 private:
  friend class AggregateSystem;
};

class AggregateSystem : public AmoebotSystem  {
  friend class ConvexHullMeasure;
  friend class SEDMeasure;
  friend class DispersionMeasure;
  friend class ClusterFractionMeasure;

 public:
  // Constructs a system of AggregateParticles with an optionally specified size
  // (#particles), form of noise (mode), and amount/value of noise (noiseVal).
  // The spread of the particles in the system is limited by a box whose
  // dimensions are based off of the number of particles available in the system
  // (with a lower bound of 50 for the box dimensions)
  AggregateSystem(int numParticles = 2, QString mode = "d",
                  double noiseVal = 3.0);

  // Checks whether or not the system's run of the aggregation algorithm has
  // terminated. By default, always returns false. Edit the function in
  // aggregation.cpp to instead use one of the metric-based stopping conditions
  // defined there.
  bool hasTerminated() const override;

  // Used when a stopping condition is active. Represents the most recent value
  // of the metric used in the specified stopping condition.
//  double currentval;

private:
  // Depth-first search (DFS) for the cluster fraction metric.
  void DFS(AggregateParticle& particle, const AggregateSystem& system,
           std::vector<AggregateParticle>& clusterVec);

};


// Calculates distance between two points. For each point, ccepts a
// QVector<double> containing the individual coordinates (does not accept
// AggregateParticle as parameter).
double dist(const QVector<double> a, const QVector<double> b);


// Returns the perimeter of the convex hull of the system
class ConvexHullMeasure : public Measure {

public:
  ConvexHullMeasure(const QString name, const unsigned int freq,
                       AggregateSystem& system);

  double calculate() const final;

protected:
  AggregateSystem& _system;
};


// Helper functions for SEDMeasure
struct Circle;
bool isInside(const Circle& c, const QVector<double> p);
Circle circleFromThree(const QVector<double> a, const QVector<double> b,
                       const QVector<double> c);
Circle circleFromTwo(const QVector<double> a, const QVector<double> b);
bool isValidCircle(const Circle& c, const QVector< QVector<double> > points);

// Returns the circumference of the smallest enclosing disc(SED) of the system
class SEDMeasure : public Measure {

public:
  SEDMeasure(const QString name, const unsigned int freq,
                       AggregateSystem& system);

  double calculate() const final;

protected:
  AggregateSystem& _system;
};


// Returns the dispersion (2nd moment) value of the system. Dispersion is
// defined as the average distance between all particles and the centroid
// (x avg, Yavg) of the system.
class DispersionMeasure : public Measure {

public:
  DispersionMeasure(const QString name, const unsigned int freq,
                    AggregateSystem& system);

  double calculate() const final;

protected:
  AggregateSystem& _system;
};


// Returns the cluster fraction value of the system. Cluster fraction is defined
// as the fraction of the system's particles that are connected to the largest
// (by number of particles) cluster of the system.
class ClusterFractionMeasure : public Measure {

public:
  ClusterFractionMeasure(const QString name, const unsigned int freq,
                         AggregateSystem& system);

  double calculate() const final;

protected:
  AggregateSystem& _system;
};

#endif  // AMOEBOTSIM_ALG_AGGREGATION_H
