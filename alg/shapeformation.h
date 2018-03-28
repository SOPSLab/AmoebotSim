// Defines the particle system and composing particles for the General
// Formation Algorithm as alluded to in 'An Algorithmic Framework for Shape
// Formation Problems in Self-Organizing Particle Systems'
// [arxiv.org/abs/1504.00744].
//
// Run with shapeformation(#particles, hole probability, mode)
// on the simulator command line.
// mode == "h" --> hexagon formation
// mode == "t1" --> vertex triangle formation
// mode == "t2" --> center triangle formation
// mode == "s" --> square formation

// TODO: 1)  change to strings DONE
//       2)  change name to shape formation and ShapeFormationParticle DONE
//       3)  make sure indentation matches throughout (Kinda done)
//       4)  consistency with neighbor vs nbr DONE (?)
//       5)  make sure the ShapeFormationsystem comments account for all parameters
//           and double check all other ocmmnets too
//       6)  Can remove the return statements in activate function DONE
//       7)  Remove brittle coding Done
//       8)  Rename to vertex triangle and center triangle Done
//       9)  When removing shape formation algorithms, need to update so that the
//           simulator initializes with the general shape formation algorithm in
//           the scriptinterface constructor
//       10) Determine where all of the legacy components are in the simulator
//       11) Take a look at the leader election algorithm
//       12) boundedobjcoating, leader election, [ring], and ShapeFormation coating

#ifndef AMOEBOTSIM_ALG_SHAPEFORMATION_H
#define AMOEBOTSIM_ALG_SHAPEFORMATION_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class ShapeFormationParticle : public AmoebotParticle {
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
  // for its local compass, a system which it belongs to, an initial state, a
  // signal for determining turning directions (currently for vertex triangle
  // and square construction), and a string to determine what shape to form.
  ShapeFormationParticle(const Node head, const int globalTailDir,
                         const int orientation, AmoebotSystem& system,
                         State state, int turn, const QString modeBit);

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
  ShapeFormationParticle& nbrAtLabel(int label) const;

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
  int moveDir;
  int followDir;
  int turnSignal;
  QString mode;

 private:
  friend class ShapeFormationSystem;
};

class ShapeFormationSystem : public AmoebotSystem  {
 public:
  // Constructs a system of ShapeFormationParticles with an optionally specified
  // size (#particles), hole probability, and optionally specified shape to
  // form. holeProb in [0,1] controls how "spread out" the system is; closer to
  // 0 is more compressed, closer to 1 is more expanded.
  // The current shapes accepted are...
  //   "h" --> hexagon
  //   "t1" --> center triangle
  //   "t2" --> vertex triangle
  //   "s" --> square
  ShapeFormationSystem(int numParticles = 200, float holeProb = 0.2,
                       QString modeBit = 0);

  // Checks whether or not the system's run of the ShapeFormation formation
  // algorithm has terminated (all particles in state Finish).
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_SHAPEFORMATION_H
