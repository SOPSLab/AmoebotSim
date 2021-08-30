/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the noisy, discrete
// adaptation of the swarm aggregation algorithm as defined in 'Deadlock and
// Noise in Self-Organized Aggregation Without Computation'
// [arxiv.org/abs/2108.09403].
//
// Two forms of noise:
// 1) Deadlock Perturbation (mode = "d") - If blocked when attempting to move
// around the center of rotation (deadlock state), rotate in place to break
// symmetry. noiseVal (integer > 0) represents the number of consecutive
// deadlock state activations needed before perturbing.
// 2) Error Probability (mode = "e") - checkIfParticleSight() function outputs
// the incorrect reading at a certain probability p. noiseVal (double in [0,1])
// represents this error probability p.
// Note: to achieve an algorithm with zero noise, use mode="e" and noiseVal=0.

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
  // particle uses is determined by the cone starting from (center + 4) % 6 (not
  // included) and ending at (center + 5) % 6 (included).
  bool checkIfParticleInSight() const;

 protected:
  int center;
  QString mode;
  double noiseVal;
  std::vector<AggregateParticle*> particles;
  int perturb;
  bool visited;

 private:
  friend class AggregateSystem;
};

class AggregateSystem : public AmoebotSystem  {
  friend class SEDMeasure;
  friend class ConvexHullMeasure;
  friend class DispersionMeasure;
  friend class ClusterFractionMeasure;

 public:
  // Constructs a system of AggregateParticles with an optionally specified size
  // (#particles), form of noise (mode), and amount/value of noise (noiseVal).
  AggregateSystem(int numParticles = 2, QString mode = "d",
                  double noiseVal = 3.0);

  // Checks whether or not the system's run of the aggregation algorithm has
  // terminated. Returns false by defualt.
  bool hasTerminated() const override;

 private:
  // Depth-first search (DFS) used in the cluster fraction metric.
  void DFS(AggregateParticle& particle, const AggregateSystem& system,
           std::vector<AggregateParticle>& clusterVec);
};

// Returns the Euclidian distance between two points.
double dist(const QVector<double> a, const QVector<double> b);

/* Copyright (c) 2020 Project Nayuki
 * https://www.nayuki.io/page/smallest-enclosing-circle
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program (see COPYING.txt and COPYING.LESSER.txt).
 * If not, see <http://www.gnu.org/licenses/>. */

// Helper functions for SEDMeasure.
struct Circle;
bool isInside(const Circle& c, const QVector<double> p);
Circle circleFromThree(const QVector<double> a, const QVector<double> b,
                       const QVector<double> c);
Circle circleFromTwo(const QVector<double> a, const QVector<double> b);
bool isValidCircle(const Circle& c, const QVector< QVector<double> > points);

// Returns the circumference of the smallest enclosing disc (SED) of the system.
class SEDMeasure : public Measure {
 public:
  SEDMeasure(const QString name, const unsigned int freq,
             AggregateSystem& system);

  double calculate() const final;

 protected:
  AggregateSystem& _system;
};

// Returns the perimeter of the convex hull of the system.
class ConvexHullMeasure : public Measure {
 public:
  ConvexHullMeasure(const QString name, const unsigned int freq,
                    AggregateSystem& system);

  double calculate() const final;

 protected:
  AggregateSystem& _system;
};

// Returns the dispersion (2nd moment) value of the system. Dispersion is
// defined as the sum of distances between all particles and the centroid
// (x avg, y avg) of the system.
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
