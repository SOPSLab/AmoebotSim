// Algorithm to construct the convex hull.
//
// Run with convexhull(#particles, #tiles) on the simulator command line.

#ifndef AMOEBOTSIM_ALG_CONVEXHULL_H
#define AMOEBOTSIM_ALG_CONVEXHULL_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class ConvexHullParticle : public AmoebotParticle {
 public:
  enum class State {
    Object,
    LeaderStart,
    LeaderWait,
    LeaderMove,
    Start,
    Wait,
    Tree,
    Follow,
    Done
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and an initial state.
  ConvexHullParticle(const Node head, const int globalTailDir,
                  const int orientation, AmoebotSystem& system, State state);

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

  virtual std::vector<int> getConvexHullApproximate() const;

 protected:
  State state;
  int parent;
  int moveDir;

  // Used to update the distance counters according to the movements
  std::vector<std::vector<int>> delta;

  // Stores the distance to each halfplane. Must be replaced by tokens eventually.
  std::vector<int> distance;

  // Used for detecting termination.
  std::vector<int> completed;
  int lastHp;

 private:
  friend class ConvexHullSystem;
};

class ConvexHullSystem : public AmoebotSystem  {
 public:
  ConvexHullSystem(int numParticles = 50, int numTiles = 300, float holeProb = 0.2);

  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_CONVEXHULL_H
