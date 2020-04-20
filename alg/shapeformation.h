/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the General
// Formation Algorithm as alluded to in 'An Algorithmic Framework for Shape
// Formation Problems in Self-Organizing Particle Systems'
// [arxiv.org/abs/1504.00744].
//
// mode == "h" --> hexagon formation
// mode == "s" --> square formation
// mode == "t1" --> vertex triangle formation
// mode == "t2" --> center triangle formation
// mode == "l" --> line formation

#ifndef AMOEBOTSIM_ALG_SHAPEFORMATION_H_
#define AMOEBOTSIM_ALG_SHAPEFORMATION_H_

#include <set>

#include <QString>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

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
  // for its local compass, a system which it belongs to, an initial state, and
  // a string to determine what shape to form.
  ShapeFormationParticle(const Node head, const int globalTailDir,
                         const int orientation, AmoebotSystem& system,
                         State state, const QString mode);

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
  QString mode;
  int turnSignal;
  int constructionDir;
  int moveDir;
  int followDir;

 private:
  friend class ShapeFormationSystem;
};

class ShapeFormationSystem : public AmoebotSystem  {
 public:
  // Constructs a system of ShapeFormationParticles with an optionally specified
  // size (#particles), hole probability, and shape to form. holeProb in [0,1]
  // controls how "spread out" the system is; closer to 0 is more compressed,
  // closer to 1 is more expanded. The current shapes accepted are...
  //   "h"  --> hexagon
  //   "s"  --> square
  //   "t1" --> vertex triangle
  //   "t2" --> center triangle
  //   "l"  --> line
  ShapeFormationSystem(int numParticles = 200, double holeProb = 0.2,
                       QString mode = "h");

  // Checks whether or not the system's run of the ShapeFormation formation
  // algorithm has terminated (all particles in state Finish).
  bool hasTerminated() const override;

  // Returns a set of strings containing the current accepted modes of
  // Shapeformation.
  static std::set<QString> getAcceptedModes();
};

#endif  // AMOEBOTSIM_ALG_SHAPEFORMATION_H_
