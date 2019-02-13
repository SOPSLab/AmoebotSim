// Defines the particle system and composing particles for the General
// Leader Election Algorithm as alluded to in 'Leader Election and Shape
// Formation with Self-Organizing Programmable Matter'
// [https://arxiv.org/abs/1503.07991].
//

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

  // Returns the borderColors and borderPointColors arrays associated with the
  // particle to draw the cycle for leader election.
  virtual std::array<int, 18> borderColors() const;
  virtual std::array<int, 6> borderPointColors() const;

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

  int getNextAgentDir(const int agentDir) const;
  int getPrevAgentDir(const int agentDir) const;

  int getNumberOfNbrs() const;

 protected:
  struct LeaderElectionToken : public Token {
   int origin;
  };

  // Tokens for Candidate Elimination via Segment Comparison
  struct SegmentLeadToken : public LeaderElectionToken {
    SegmentLeadToken(int origin = -1) {
      this->origin = origin;
    }
  };
  struct PassiveSegmentToken : public LeaderElectionToken {
    bool isFinal;
    PassiveSegmentToken(int origin = -1, bool isFinal = false) {
      this->origin = origin;
      this->isFinal = isFinal;
    }
  };
  struct ActiveSegmentToken : public LeaderElectionToken {
    bool isFinal;
    ActiveSegmentToken(int origin = -1, bool isFinal = false) {
      this->origin = origin;
      this->isFinal = isFinal;
    }
  };
  struct PassiveSegmentCleanToken : public LeaderElectionToken {
    PassiveSegmentCleanToken(int origin = -1) {
      this->origin = origin;
    }
  };
  struct ActiveSegmentCleanToken : public LeaderElectionToken {
    ActiveSegmentCleanToken(int origin = -1) {
      this->origin = origin;
    }
  };
  struct FinalSegmentCleanToken : public LeaderElectionToken {
    bool hasCoveredCandidate;
    FinalSegmentCleanToken(int origin = -1, bool hasCovered = false) {
      this->origin = origin;
      this->hasCoveredCandidate = hasCovered;
    }
  };

  // Tokens for Coin Flipping and Candidate Transferal
  struct CandidacyAnnounceToken : public LeaderElectionToken {
    CandidacyAnnounceToken(int origin = -1) {
      this->origin = origin;
    }
  };
  struct CandidacyAckToken : public LeaderElectionToken {
    CandidacyAckToken(int origin = -1) {
      this->origin = origin;
    }
  };

  // Tokens for Solitude Verification
  struct SolitudeActiveToken : public LeaderElectionToken {
    SolitudeActiveToken(int origin = -1) {
      this->origin = origin;
    }
  };
  struct SolitudePositiveXToken : public LeaderElectionToken {
    SolitudePositiveXToken(int origin = -1) {
      this->origin = origin;
    }
  };
  struct SolitudePositiveYToken : public LeaderElectionToken {
    SolitudePositiveYToken(int origin = -1) {
      this->origin = origin;
    }
  };
  struct SolitudeNegativeXToken : public LeaderElectionToken {
    SolitudeNegativeXToken(int origin = -1) {
      this->origin = origin;
    }
  };
  struct SolitudeNegativeYToken : public LeaderElectionToken {
    SolitudeNegativeYToken(int origin = -1) {
      this->origin = origin;
    }
  };

  // Token for Border Testing
  struct BorderTestToken : public LeaderElectionToken {};
 private:
  friend class LeaderElectionSystem;
  class LeaderElectionAgent {
  public:
   enum class SubPhase {
     SegmentComparison = 0,
     CoinFlipping,
     SolitudeVerification,
     BoundaryTesting
   };

   LeaderElectionAgent();

   int localId;
   int agentDir, nextAgentDir, prevAgentDir;
   State agentState;
   SubPhase subPhase;
   LeaderElectionParticle* candidateParticle;
   // Variables for Segment Comparison
   bool comparingSegment = false;
   bool isCoveredCandidate = false;
   bool absorbedActiveToken = false;

   // Variables for Coin Flipping and Candidacy Transferral
   bool gotAnnounceInCompare = false;
   bool gotAnnounceBeforeAck = false;
   bool waitingForTransferAck = false;

   void activate();

   void activeClean(int agentDir);
   void passiveClean(int agentDir);

   template <class TokenType>
   bool hasAgentToken(int agentDir);
   template <class TokenType>
   std::shared_ptr<TokenType> peekAgentToken(int agentDir);
   template <class TokenType>
   std::shared_ptr<TokenType> takeAgentToken(int agentDir);
   template <class TokenType>
   void passAgentToken(int agentDir);
   template <class TokenType>
   void passAgentToken(int agentDir, bool opt);

   void setStateColor();
   void setSubPhaseColor();

   // Methods responsible for painting the borders which will act as physical
   // representations of the cycle for leader election
   void paintFrontSegment(const int color);
   void paintBackSegment(const int color);
  };

 protected:
  State state;
  unsigned currentAgent;
  std::vector<LeaderElectionAgent*> agents;
  std::array<int, 18> borderColorLabels;
  std::array<int, 6> borderPointColorLabels;
};

class LeaderElectionSystem : public AmoebotSystem {
 public:
  // Constructs a system of ShapeFormationParticles with an optionally specified
  // size (#particles), hole probability, and shape to form. holeProb in [0,1]
  // controls how "spread out" the system is; closer to 0 is more compressed,
  // closer to 1 is more expanded.
  LeaderElectionSystem(int numParticles = 100, double holeProb = 0.2);

  // Checks whether or not the system's run of the ShapeFormation formation
  // algorithm has terminated (all particles in state Finish).
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_LEADERELECTION_H
