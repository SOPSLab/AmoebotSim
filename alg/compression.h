// Defines the particle system and composing particles for the Compression
// Algorithm as defined in 'A Markov Chain Algorithm for Compression in
// Self-Organizing Particle Systems' [arxiv.org/abs/1603.07991]. In particular,
// this simulates the local, distributed, asynchronous algorithm A using the
// #neighbors metric instead of the #triangles metric.
//
// Run with compression(#particles, hole probability) on the simulator command
// line.

#ifndef AMOEBOTSIM_ALG_COMPRESSION_H
#define AMOEBOTSIM_ALG_COMPRESSION_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class CompressionParticle : public AmoebotParticle {
 public:
  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and a bias parameter.
  CompressionParticle(const Node head, const int globalTailDir,
                      const int orientation, AmoebotSystem& system,
                      const float lambda);

  // Executes one particle activation.
  virtual void activate();

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  virtual QString inspectionText() const;

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  CompressionParticle& nbrAtLabel(int label) const;

protected:
  // Particle memory.
  const float lambda;
  float q;
  int numNbrsBefore;
  bool flag;

private:
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

  friend class CompressionSystem;
};

class CompressionSystem : public AmoebotSystem {
 public:
  // Constructs a system of CompressionParticles connected to a randomly
  // generated surface (with no tunnels). Takes an optionally specified size
  // (#particles) and a bias parameter. A bias above 2 + sqrt(2) will provably
  // yield compression; a bias below 2.17 will provably yield expansion.
  CompressionSystem(int numParticles = 100, float lambda = 4.0);

  // Because this algorithm never terminates, this simply returns false.
  virtual bool hasTerminated() const;
};

#endif  // AMOEBOTSIM_ALG_COMPRESSION_H
