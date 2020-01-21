// Defines the particle system and composing particles for the Improved
// Leader Election Algorithm as alluded to in 'Improved Leader Election for
// Self-Organizing Programmable Matter'
// [https://arxiv.org/abs/1701.03616].
//
// A side remark about the algorithm is about an additional condition that may
// cause the algorithm to fail to elect a leader (apart from the probability
// that all agents elect to demote themselves in the Segment Setup phase):
// In the Segment Setup phase, if the agents determine their agent states in
// such a way that each candidate agent is either 1 demoted agent away from
// one another, or directly next to each other, the algorithm will not progress
// beyond the Identifier Comparison phase and fail to elect a leader. This may
// be observed in cycles of smaller lengths (such as ones in the inner
// boundary).

#ifndef AMOEBOTSIM_ALG_IMPROVEDLEADERELECTION_H
#define AMOEBOTSIM_ALG_IMPROVEDLEADERELECTION_H

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

#include <set>
#include <QString>
#include <QDebug>

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
   // for its local compass, and a system which it belongs to.
  LeaderElectionParticle(const Node head, const int globalTailDir,
                         const int orientation, AmoebotSystem& system,
                         State state);

  // Executes one particle activation.
  virtual void activate();

  // Functions for altering a particle's cosmetic appearance; headMarkColor
  // (respectively, tailMarkColor) returns the color to be used for the ring
  // drawn around the head (respectively, tail) node. Tail color is not shown
  // when the particle is contracted.
  virtual int headMarkColor() const;
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
  // token under the Improved Leader Election algorithm behaves.
  struct LeaderElectionToken : public Token {
    // origin is used to define the direction (label) from which a
    // LeaderElectionToken has been sent from.
   int origin;
  };

  // Tokens for Identifier Setup
  struct SetUpToken : public LeaderElectionToken {
    int comparisonColor = -1;
    SetUpToken(int comparisonColor = -1, int origin = -1) {
      this->comparisonColor = comparisonColor;
      this->origin = origin;
    }
  };

  struct NextIDPassToken : public LeaderElectionToken {
    int val = -1;
    int comparisonColor = -1;
    NextIDPassToken(int origin = -1, int val = -1, int comparisonColor = -1) {
      this->origin = origin;
      this->val = val;
      this->comparisonColor = comparisonColor;
    }
  };

  struct PrevIDPassToken : public LeaderElectionToken {
    int val = -1;
    PrevIDPassToken(int origin = -1, int val = -1) {
      this->origin = origin;
      this->val = val;
    }
  };

  // Tokens for Identifier Comparison
  struct DigitToken : public LeaderElectionToken {
    int value = -1;
    bool isActive = false;
    DigitToken(int origin = -1, int value = -1, bool isActive = false) {
      this->origin = origin;
      this->value = value;
      this->isActive = isActive;
    }
  };

  // DelimiterToken carries the comparison values from the previous comparisons
  // of agents with DigitTokens to gauge the compare the id segments of the
  // different candidate agnets. The value stored in compare follows the same
  // guidelines as in compareStatus (see below).
  struct DelimiterToken : public LeaderElectionToken {
    int value = 0;
    bool isActive = false;
    int compare = 0;
    int comparisonColor = -1;
    DelimiterToken(int origin = -1, int comparisonColor = -1,
                   int value = -1, bool isActive = false, int compare = 0) {
      this->origin = origin;
      this->comparisonColor = comparisonColor;
      this->value = value;
      this->isActive = isActive;
      this->compare = compare;
    }
  };

  // Tokens for Solitude Verification
  struct SolitudeActiveToken : public LeaderElectionToken {
    bool isSoleCandidate;
    std::pair<int, int> vector;
    int local_id;
    SolitudeActiveToken(int origin = -1,
                        std::pair<int, int> vector = std::make_pair(1, 0),
                        int local_id = -1,
                        bool isSole = true) {
      this->origin = origin;
      this->vector = vector;
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
  friend class ImprovedLeaderElectionSystem;

  // The nested class LeaderElectionAgent is used to define the behavior for the
  // agents as described in the paper
  class LeaderElectionAgent {
  public:
   enum class SubPhase {
     SegmentSetup = 0,
     IdentifierSetup,
     IdentifierComparison,
     SolitudeVerification
   };

   LeaderElectionAgent();

   // General variables in agent memory:
   // The particle emulating this agent assigns it a localId in [1,3] to
   // distinguish it from the other agents it may be emulating. From the
   // particle's perspective, this agent is in local direction/label agentDir.
   // The neighboring particle emulating the next (respectively, previous)
   // agent on this agent's boundary is in local direction nextAgentDir
   // (respectively, prevAgentDir). passTokensDir is used to determine if the
   // agent should pass tokens toward nextAgentDir (if 0) or prevAgentDir (if
   // 1). This is done to maintain the rule from direct write communication
   // that a particle can only write into the memory of one of its neighbors in
   // a single activation. demotedFromComparison is used to determine whether or
   // not the current agent was originally a candidate, which may impact its
   // behavior for phases such as Identifier Comparison.
   int localId;
   int agentDir, nextAgentDir, prevAgentDir;
   int passTokensDir = -1;
   bool demotedFromComparison = false;

   State agentState;
   SubPhase subPhase;
   LeaderElectionParticle* candidateParticle;

   // Variables for Identifier Setup:
   // hasGeneratedSetupToken is used to determine whether or not the candidate
   // agent has generated a SetupToken.
   // hasGeneratedReverseToken is used to determine whether or not the agent
   // has generated an id token for the corresponding candidate's segment.
   bool hasGeneratedSetupToken = false;
   bool hasGeneratedReverseToken = false;

   // Variables for Identifier Comparison:
   // compareStatus is used to store the difference between the current agent
   // and the DigitToken that it has matched with. compareStatus is computed as
   // the difference between the agent's digit value and the DigitToken's value,
   // i.e., compareStatus = idValue - DigitToken.value, with the values
   // representing the following relationships:
   // -1 --> agent value less than token value
   // 0 --> agent value equal to token value
   // 1 --> agent value greater than token value
   // idValue is used to store the agent's id value generated from the
   // Identifier Setup Phase.
   // comparisonColor is used to store the agent's identifying color for the
   // Identifier Comparison Phase.
   // isActive is used to determine whether or not the agent has matched with a
   // DigitToken.
   int compareStatus = -1;
   int idValue = -1;
   int comparisonColor = randInt(0, 16777216);
   bool isActive = false;

   // canPassComparisonToken is a helper functino for the Identifier Comparison
   // phase to determine whether or not the current agent may pass the
   // Identifier Comparison token (Digit Token or Delimiter Token, determined
   // by the boolean parameter).
   bool canPassComparisonToken(bool isDelimiter) const;

   // Variables for Solitude Verification
   // createdLead is true if this agent generated a solitude active token and
   // passed it forward during Solitude Verification.
   // hasGeneratedTokens is true if this agent generated solitude vector tokens
   // using the solitude active token. This is used to avoid incorrectly mixing
   // tokens of different agents on the same particle in Solitude Verification.
   bool createdLead = false;
   bool hasGeneratedTokens = false;

   // Variables for Boundary Testing
   // testingBorder is true if this agent is the sole candidate and has begun
   // the Boundary Testing subphase.
   bool testingBorder = false;

   // The activate function is the LeaderElectionAgent equivalent of an
   // Amoebot Particle's activate function.
   void activate();

   void cleanAllTokens();

   // Solitude Verification Methods
   // augmentDirVector takes a <int, int> pair as a parameter, which represents
   // the current vector stored in the solitude active token. This function
   // then generates the next vector according to a local coordinate system
   // (which is determined when a candidate agent in the Solitude Verification
   // subphase generates the solitude active token) based on the vector stored
   // in the solitude active token.
   std::pair<int, int> augmentDirVector(std::pair<int, int> vector);

   // generateSolitudeVectorTokens generates the solitude vector tokens
   // (SolitudePositiveXToken, SolitudeNegativeXToken, etc.) based on the given
   // parameter vector.
   void generateSolitudeVectorTokens(std::pair<int, int> vector);

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
   // front and back segments gray.
   void cleanSolitudeVerificationTokens();

   // Boundary Testing methods
   int addNextBorder(int currentSum) const;

   // Methods for passing, taking, and checking the ownership of tokens at the
   // agent level.
   template <class TokenType>
   bool hasAgentToken(int agentDir) const;
   template <class TokenType>
   std::shared_ptr<TokenType> peekAgentToken(int agentDir) const;
   template <class TokenType>
   std::shared_ptr<TokenType> takeAgentToken(int agentDir);
   template <class TokenType>
   void passAgentToken(int agentDir, std::shared_ptr<TokenType> token);

   // Method for counting the number of tokens at the agent level.
   template <class TokenType>
   int countAgentTokens(int agentDir) const;

   LeaderElectionAgent* nextAgent() const;
   LeaderElectionAgent* prevAgent() const;

   // Methods responsible for rendering the agents onto the simulator with their
   // colors changing based on the state and the subphase of the current agent.
   // Yellow --> Identifier Setup
   // Random Color (determined by comparisonColor) --> Identifier Comparison
   // Blue --> Solitude Verification Subphase
   // Grey --> Demoted agent
   // Green --> Sole candidate
   void setStateColor();
   void setSubPhaseColor();

   // Methods responsible for painting the borders which will act as physical
   // representations of the cycle for leader election.
   // Yellow --> Identifier Setup
   // Random Color (determined by comparisonColor) --> DelimiterToken for
   // Identifier Comparison
   // Blue --> Solitude Verification Phase
   // Grey --> No specific phase
   void paintFrontSegment(const int color);
   void paintBackSegment(const int color);
  };

 protected:
  State state;
  unsigned currentAgent;
  std::vector<LeaderElectionAgent*> agents;
  std::array<int, 18> borderColorLabels;
  std::array<int, 6> borderPointColorLabels;

  // leaderSelected is used to act as a signal for when a leader is selected to
  // set all of the remaining particles to finished.
  bool leaderSelected;
};

class ImprovedLeaderElectionSystem : public AmoebotSystem {
 public:
  // Constructs a system of LeaderElectionParticles with an optionally specified
  // size (#particles), hole probability, and shape to form. holeProb in [0,1]
  // controls how "spread out" the system is; closer to 0 is more compressed,
  // closer to 1 is more expanded.
  ImprovedLeaderElectionSystem(int numParticles = 100, double holeProb = 0.2);

  // Checks whether or not the system's run of the Leader Election algorithm has
  // terminated (all particles in state Finished or Leader).
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_IMPROVEDLEADERELECTION_H
