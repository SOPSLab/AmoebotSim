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
    Idle,
    Leader,
    LeaderWait,
    LeaderOrtho,
    LeaderDone,
    Follower,
    FollowerWait,
    FollowerWait2,
    FollowerDone,
    FollowerOrtho
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and an initial state.
  ConvexHullParticle(const Node head, const int globalTailDir,
                  const int orientation, AmoebotSystem& system, State state);

  // Executes one particle activation.
  virtual void activate();

  // Updates the distance vector according to move direction
  void updateDistances(int dir);

  // Updates the completed vector according to move direction
  void updateCompleted(int dir);

  // Returns the label of the first port incident to a neighboring particle in
  // any of the specified states, starting at the (optionally) specified label
  // and continuing clockwise.
  int labelOfFirstNbrInState(std::initializer_list<State> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNeighborInState(std::initializer_list<State> states) const;

  // Checks whether there is a hull neighbor in the given direction''
  bool hasHullParticleInDir(int dir) const;

  // Checks if the neighbor in a given direction is in a certain state
  bool nbrInDirIsInState(int dir, std::initializer_list<State> states) const;

  // Function called by the leader to swap itself with a tree particle
  void swapWithFollowerInDir(int dir);

  // Checks whether the particle has a child (an adjacent tree particle that point at it)
  bool hasChild() const;

  // Checks whether all children are already done
  bool allChildrenDone() const;

  // Pulls in a child, if possible
  void pullChildIfPossible();

  // Pushes a parent, if possible (might be the leader)
  void pushParentIfPossible();

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

 protected:
  State state;
  int parentDir;
  int moveDir;

  // Used to update the distance counters according to the movements
  std::vector<std::vector<int>> delta;

  // Stores the distance to each halfplane. Must be replaced by tokens eventually.
  std::vector<int> distance;

  // Used for detecting termination of phase 1
  std::vector<int> completed;

  // Used for detecting termination of phase 2
  int turns;

  // Flag to detect whether the particle lies on the hull after contraction
  bool preHull;

  // Flag to detect whether the particle is on the hull
  bool onHull;

  // Successor and Predecessor pointers for ortho-convex hull
  int successor;
  int predecessor;

 private:
  friend class ConvexHullSystem;
};

class ConvexHullSystem : public AmoebotSystem  {
 public:
  ConvexHullParticle* leader;

  ConvexHullSystem(int numParticles = 50, int numTiles = 300, float holeProb = 0.2);

  virtual std::vector<Node> getConvexHullApproximate() const;

  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_CONVEXHULL_H
