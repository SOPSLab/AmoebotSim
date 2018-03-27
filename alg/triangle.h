// Defines the particle system and composing particles for the Triangle
// Formation Algorithm as defined in 'An Algorithmic Framework for Shape
// Formation Problems in Self-Organizing Particle Systems'
// [arxiv.org/abs/1504.00744].
//
// Run with triangle(# of particles, hole probability, mode) in the simulator
// command line

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class TriangleParticle : public AmoebotParticle {
public:
  enum class State {
    Seed,
    Idle,
    Follow,
    Lead,
    Finish
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and an initial state.
  TriangleParticle(const Node head, const int globalTailDir,
               const int orientation, AmoebotSystem& system, State state,
               const int mode, int turn);

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
  TriangleParticle& nbrAtLabel(int label) const;

  // Returns the label of the first port incident to a neighboring particle in
  // any of the specified states, starting at the (optionally) specified label
  // and continuing clockwise.
  int labelOfFirstNbrInState(std::initializer_list<State> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<State> states) const;

  // Returns the label of the port incident to a neighbor which is finished and
  // pointing at this particle's position as the next one to be filled; returns
  // -1 if such a neighbor does not exist.
  int constructionReceiveDir() const;

  // Checks whether this particle is occupying the next position to be filled.
  bool canFinish() const;

  // Sets this particle's constructionDir to point at the next position to be
  // filled as it is finishing.
  void updateConstructionDir();

  // Updates this particle's moveDir when it is a leader to traverse the current
  // surface of the forming shape counter-clockwise.
  void updateMoveDir();

  // Checks whether this particle has an immediate child in the spanning tree
  // following its tail.
  bool hasTailFollower() const;

protected:
  State state;
  int constructionDir;
  int followDir;
  int moveDir;
  int modeBit;
  int turnSignal;

private:
  friend class TriangleSystem;
};

class TriangleSystem : public AmoebotSystem {
  public:

    // Constructs a system of TriangleParticles with an optionally specified size
    // (#particles) and hole probability. holeProb in [0,1] controls how "spread
    // out" the system is; closer to 0 is more compressed, closer to 1 is more
    // expanded.
    TriangleSystem(int numParticles = 200, float holeProb = 0.2, int mode = 0);

    // Checks whether or not the system's run of the Triangle formation algorithm
    // has terminated (all particles in state Finish).
    bool hasTerminated() const override;
};

#endif // TRIANGLE_H
