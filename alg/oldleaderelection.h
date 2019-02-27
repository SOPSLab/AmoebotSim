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
  // particle to draw the boundaries for leader election.
  virtual std::array<int, 18> borderColors() const;
  virtual std::array<int, 6> borderPointColors() const;

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  LeaderElectionParticle& nbrAtLabel(int label) const;

  // Returns the label associated with the direction which the next (resp.
  // previous) agent is according to the cycle that the agent is on (which is
  // determined by the provided agentDir parameter)
  int getNextAgentDir(const int agentDir) const;
  int getPrevAgentDir(const int agentDir) const;

  // Returns a count of the number of particle neighbors surrounding the calling
  // particle.
  int getNumberOfNbrs() const;

 protected:
  // The LeaderElectionToken struct provides a general framework of what a
  // token under the General Leader Election algorithm behaves.
  // origin is used to define the direction (label) from which a
  // LeaderElectionToken has been sent from.
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
    bool isSoleCandidate = false;
    int generatedVector;
    int local_id;
    SolitudeActiveToken(int origin = -1, int vector = -1, int local_id = -1,
                        bool isSole = false) {
      this->origin = origin;
      this->generatedVector = vector;
      this->local_id = local_id;
      this->isSoleCandidate = isSole;
    }
  };
  struct SolitudeVectorToken : public LeaderElectionToken {
    bool isSettled;
  };

  struct SolitudePositiveXToken : public SolitudeVectorToken {
    SolitudePositiveXToken(int origin = -1, bool settled = false) {
      this->origin = origin;
      this->isSettled = settled;
    }
  };
  struct SolitudePositiveYToken : public SolitudeVectorToken {
    SolitudePositiveYToken(int origin = -1, bool settled = false) {
      this->origin = origin;
      this->isSettled = settled;
    }
  };
  struct SolitudeNegativeXToken : public SolitudeVectorToken {
    SolitudeNegativeXToken(int origin = -1, bool settled = false) {
      this->origin = origin;
      this->isSettled = settled;
    }
  };
  struct SolitudeNegativeYToken : public SolitudeVectorToken {
    SolitudeNegativeYToken(int origin = -1, bool settled = false) {
      this->origin = origin;
      this->isSettled = settled;
    }
  };

  // Token for Border Testing
  struct BorderTestToken : public LeaderElectionToken {
    int borderSum;
    BorderTestToken(int origin = -1, int borderSum = -1) {
      this->origin = origin;
      this->borderSum = borderSum;
    }
  };
 private:
  friend class LeaderElectionSystem;

  // The nested class LeaderElectionAgent is used to define the behavior for the
  // agents as described in the paper
  class LeaderElectionAgent {
  public:
   enum class SubPhase {
     SegmentComparison = 0,
     CoinFlipping,
     SolitudeVerification
   };

   LeaderElectionAgent();

   // localId is an int which stores which id an agent has according to the
   // particle which is associated with it. This localId value lies in [1,3]
   int localId;
   // agentDir stores the direction/label of the current agent according to the
   // labelling scheme used by the particle that owns the current agent.
   // nextAgentDir and prevAgentDir are store the direction/label of the next
   // and previous (respectively) agents of the current agent according to the
   // particle that owns the current agent and the direction of the boundary
   // that the agent is on.
   int agentDir, nextAgentDir, prevAgentDir;

   State agentState;
   SubPhase subPhase;
   LeaderElectionParticle* candidateParticle;

   // Variables for Segment Comparison
   // comparingSegment is a boolean which is used to keep track of whether or
   // not the current agent in the Segment Comparison subphase has generated
   // a segment lead token and passed it along its front segment.
   // isCoveredCandidate is a boolean which is used for candidate agents in any
   // subphase. If the candidate agent receives an active token which is from
   // the nextAgentDir, it will set this boolean to true and demote itself.
   // Afterwards, the final segment clean token that passes over a demoted agent
   // which has a isCoveredCandidate value of "true" will indicate to the
   // receiving candidate agent in the Segment Comparison subphase that it has
   // covered a candidate agent in its segment comparison phase.
   // absorbedActiveToken is a boolean which is used to keep track of whether or
   // not an agent has absorbed an active token. This is important for the
   // Segment Comparison subphase to keep track of whether or not to absorb
   // an active token or pass it along backwards according to the cycle.
   bool comparingSegment = false;
   bool isCoveredCandidate = false;
   bool absorbedActiveToken = false;

   // Variables for Coin Flipping and Candidacy Transferral
   // gotAnnounceInCompare is a boolean which is used in the Segment Comparison
   // subphase, where, in the case an agent has failed its segment comparison,
   // but has gotten a candidate announcement token during its segment
   // comparison, rather than demoting itself, the segment comparison agent will
   // move onto the next subphase.
   // gotAnnounceBeforeAck is a boolean which is used in the Coin Flipping
   // subphase. It is flipped when an agent in the Coin Flipping subphase
   // receives a candidate announcement token before it receives a candidate
   // acknowledgement token, thus indicating that the agent should not demote
   // itself and should instead move forwards to the Solitude Verification
   // subphase.
   // waitingForTransferAck is a boolean which is used to keep track of whether
   // or not the current agent in the Coin Flipping subphase has generated a
   // candidate announcement token or not.
   bool gotAnnounceInCompare = false;
   bool gotAnnounceBeforeAck = false;
   bool waitingForTransferAck = false;

   // Variables for Solitude Verification
   // createdLead is a boolean which determines whether or not the current agent
   // has generated a solitude active token and passed it along its front
   // segment.
   // hasGeneratedTokens is a boolean which determines whether or not the
   // current agent has generated solitude vector tokens using the solitude
   // active token. This is necessary since there are cases where different
   // agents on the same particle might have the same nextAgentDir as the
   // other's prevAgentDir, so an incorrect token pass is avoided using this
   // boolean variable.
   bool createdLead = false;
   bool hasGeneratedTokens = false;

   // Variables for Boundary Testing
   // testingBorder is a boolean used to determine whether or not the current
   // sole candidate has generated a border testing token to determine what
   // boundary it is on.
   bool testingBorder = false;

   // The activate function is the LeaderElectionAgent equivalent of an
   // Amoebot Particle's activate function
   void activate();

   // Methods for token cleaning if a candidate is covered.
   // The boolean parameter "first" is used to determine whether or not the
   // cleaning agent is a covered candidate which has just absorbed an active
   // token and must delete/clean its tokens for the first time.
   void activeClean(bool first);
   void passiveClean(bool first);

   // Solitude Verification Methods
   // The following functions are responsible for converting to a "local"
   // coordinate system according to the orientation of the agent which has
   // first generated the solitude active token. Afterwards, this orientation
   // is tracked according to a value stored in the SolitudeActiveToken which
   // is created and updated according to the values returned by the functions
   // below.
   int encodeVector(std::pair<int, int> vector) const;
   std::pair<int, int> decodeVector(int code);
   std::pair<int, int> augmentDirVector(std::pair<int, int> vector);

   // The checkSolitudeXTokens and checkSolitudeYTokens are used to determine
   // the condition of the solitude vector tokens that an agent might own.
   // The functions will return a value contained in [0,2] depending on what
   // condition the solitude vector tokens are in:
   // 0 --> tokens are settled and there is a mismatch, i.e., the agent might
   // have a positive x token (which as settled), but no corresponding negative
   // x token.
   // 1 --> at least one of the tokens is not settled
   // 2 --> tokens are settled and there is a match or neither tokens are
   // present on the current agent.
   int checkSolitudeXTokens() const;
   int checkSolitudeYTokens() const;

   // The cleanSolitudeVerificationTokens function will clean the solitude
   // vector tokens owned by a particular agent as well as paint the
   // front and back segments gray
   void cleanSolitudeVerificationTokens();

   // Boundary Testing methods
   int addNextBorder(int currentSum);

   // Methods for passing, taking, and checking the ownership of tokens at the
   // agent level
   template <class TokenType>
   bool hasAgentToken(int agentDir) const;
   template <class TokenType>
   std::shared_ptr<TokenType> peekAgentToken(int agentDir) const;
   template <class TokenType>
   std::shared_ptr<TokenType> takeAgentToken(int agentDir);
   template <class TokenType>
   void passAgentToken(int agentDir);
   template <class TokenType>
   void passAgentToken(int agentDir, bool opt);
   template <class TokenType>
   void passAgentToken(int agentDir, int vect, int id);
   template <class TokenType>
   void passAgentToken(int agentDir, std::shared_ptr<TokenType> token);
   LeaderElectionAgent* nextAgent() const;
   LeaderElectionAgent* prevAgent() const;

   // Methods responsible for rendering the agents onto the simulator with their
   // colors changing based on the state and the subphase of the current agent
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
