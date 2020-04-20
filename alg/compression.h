/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the Compression
// Algorithm as defined in 'A Markov Chain Algorithm for Compression in
// Self-Organizing Particle Systems' [arxiv.org/abs/1603.07991]. In particular,
// this simulates the local, distributed, asynchronous algorithm A using the
// #neighbors metric instead of the #triangles metric.

#ifndef AMOEBOTSIM_ALG_COMPRESSION_H_
#define AMOEBOTSIM_ALG_COMPRESSION_H_

#include <QString>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class CompressionParticle : public AmoebotParticle {
  friend class CompressionSystem;
  friend class PerimeterMeasure;

 public:
  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and a bias parameter.
  CompressionParticle(const Node head, const int globalTailDir,
                      const int orientation, AmoebotSystem& system,
                      const double lambda);

  // Executes one particle activation.
  virtual void activate();

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  virtual QString inspectionText() const;

protected:
  // Particle memory.
  const double lambda;
  double q;
  int numNbrsBefore;
  bool flag;

private:
  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  CompressionParticle& nbrAtLabel(int label) const;

  // hasExpNbr() checks whether this particle has an expanded neighbor, while
  // hasExpHeadAtLabel() checks whether the head of an expanded neighbor is at
  // the position at the specified label.
  bool hasExpNbr() const;
  bool hasExpHeadAtLabel(const int label) const;

  // Counts the number of neighbors in the labeled positions. Note: this
  // implicitly assumes all neighbors are unique, as none are expanded.
  int nbrCount(std::vector<int> labels) const;

  // Functions for checking Properties 1 and 2 of the compression algorithm.
  bool checkProp1(std::vector<int> S) const;
  bool checkProp2(std::vector<int> S) const;
};

class CompressionSystem : public AmoebotSystem {
  friend class PerimeterMeasure;

 public:
  // Constructs a system of CompressionParticles connected to a randomly
  // generated surface (with no tunnels). Takes an optionally specified size
  // (#particles) and a bias parameter. A bias above 2 + sqrt(2) will provably
  // yield compression; a bias below 2.17 will provably yield expansion.
  CompressionSystem(int numParticles = 100, double lambda = 4.0);

  // Because this algorithm never terminates, this simply returns false.
  virtual bool hasTerminated() const;
};

class PerimeterMeasure : public Measure {
 public:
  // Constructs a PerimeterMeasure by using the parent constructor and adding a
  // reference to the CompressionSystem being measured.
  PerimeterMeasure(const QString name, const unsigned int freq,
                   CompressionSystem& system);

  // Calculates the perimeter of the system, i.e., the number of edges on the
  // walk around the unique external boundary of the system. Uses the fact
  // that perimeter = (3 * #particles) - (#nearest neighbor pairs) - 3.
  double calculate() const final;

 protected:
  CompressionSystem& _system;
};

#endif  // AMOEBOTSIM_ALG_COMPRESSION_H_
