// Defines the particle system and composing particles for the General
// Leader Election Algorithm as alluded to in 'An Algorithmic Framework for Shape
// Formation Problems in Self-Organizing Particle Systems'
// [arxiv.org/abs/1504.00744].
//
// Run with shapeformation(#particles, hole probability, mode)
// on the simulator command line.
// mode == "h" --> hexagon formation
// mode == "s" --> square formation
// mode == "t1" --> vertex triangle formation
// mode == "t2" --> center triangle formation

#ifndef AMOEBOTSIM_ALG_LEADERELECTION_H
#define AMOEBOTSIM_ALG_LEADERELECTION_H

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

#include <set>
#include <QString>

class LeaderElectionParticle : public AmoebotParticle {
 public:
  enum class State {
    Idle,
    Candidate,
    SoleCandidate,
    Demoted,
    Leader,
    Finished
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, an initial state, a
  // signal for determining turning directions (currently for vertex triangle
  // and square construction), and a string to determine what shape to form.
  LeaderElectionParticle(const Node head, const int globalTailDir,
                         const int orientation, AmoebotSystem& system,
                         State state);

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
  LeaderElectionParticle& nbrAtLabel(int label) const;

  // Returns the label of the first port incident to a neighboring particle in
  // any of the specified states, starting at the (optionally) specified label
  // and continuing clockwise.
  int labelOfFirstNbrInState(std::initializer_list<State> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<State> states) const;

  // Checks whether this particle is occupying the next position to be filled.
  bool canFinish() const;

 protected:
  State state;
  // Tokens for Candidate Elimination via Segment Comparison
  struct SegmentLeadToken : public Token {};
  struct PassiveSegmentToken : public Token {};
  struct ActiveSegmentToken : public Token {};
  struct PassiveSegmentCleanToken : public Token {};
  struct ActiveSegmentCleanToken : public Token {};
  struct FinalSegmentCleanToken : public Token {};

  // Tokens for Coin Flipping and Candidate Transferal
  struct CandidacyAnnounceToken : public Token {};
  struct CandidacyAckToken : public Token {};

  // Tokens for Solitude Verification
  struct SolitudeActiveToken : public Token {};
  struct SolitudePositiveXToken : public Token {};
  struct SolitudePositiveYToken : public Token {};
  struct SolitudeNegativeXToken : public Token {};
  struct SolitudeNegativeYToken : public Token {};

  // Token for Border Testing
  struct BorderTestToken : public Token {};
 private:
  friend class LeaderElectionSystem;
  class LeaderElectionAgent {

  };
};

class LeaderElectionSystem : public AmoebotSystem {
 public:
  // Constructs a system of ShapeFormationParticles with an optionally specified
  // size (#particles), hole probability, and shape to form. holeProb in [0,1]
  // controls how "spread out" the system is; closer to 0 is more compressed,
  // closer to 1 is more expanded. The current shapes accepted are...
  //   "h"  --> hexagon
  //   "s"  --> square
  //   "t1" --> vertex triangle
  //   "t2" --> center triangle
  LeaderElectionSystem(int numParticles = 200, double holeProb = 0.2,
                       QString mode = "h");

  // Checks whether or not the system's run of the ShapeFormation formation
  // algorithm has terminated (all particles in state Finish).
  bool hasTerminated() const override;

  // Returns a set of strings containing the current accepted modes of
  // Shapeformation.
  static std::set<QString> getAcceptedModes();
};

#endif  // AMOEBOTSIM_ALG_LEADERELECTION_H
