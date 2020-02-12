/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the General
// Leader Election Algorithm as alluded to in 'Leader Election and Shape
// Formation with Self-Organizing Programmable Matter'
// [https://arxiv.org/abs/1503.07991].
//
// Please note that this distributed implementation of the algorithm described
// in the above paper has a chance of not working (which is related to the
// Segment Comparison subphase); however, the centralized algorithm described in
// the paper (which this distributed implementation is based on) is correct.
// Specifically, there is a possible order of activations when there are only
// two candidates in the Segment Comparison phase, where both candidates will
// demote themselves. This results in the boundary that the two candidates were
// on to lack any leader.
//
// Also note that the Solitude Verification phase of this algorithm was adapted
// from the Solitude Verification phase from the Improved Leader Election
// algorithm (see 'Improved Leader Election for Self-Organizing Programmable
// Matter' [https://arxiv.org/abs/1701.03616]). This decision was made because
// the Solitude Verification phase in the Improved Leader Election algorithm
// is significantly simpler than this version's, and both Solitude Verification
// phases accomplish the same goals.

#ifndef AMOEBOTSIM_ALG_LEADERELECTION_H_
#define AMOEBOTSIM_ALG_LEADERELECTION_H_

#include <array>
#include <vector>

#include <QString>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

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
  // when the particle is contracted. headMarkDir returns the label of the port
  // on which the black head marker is drawn.
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
  // determined by the provided agentDir parameter).
  int getNextAgentDir(const int agentDir) const;
  int getPrevAgentDir(const int agentDir) const;

  // Returns a count of the number of particle neighbors surrounding the calling
  // particle.
  int getNumberOfNbrs() const;

 protected:
  // The LeaderElectionToken struct provides a general framework of any token
  // under the General Leader Election algorithm.
  struct LeaderElectionToken : public Token {
    // origin is used to define the direction (label) that a LeaderElectionToken
    // was received from.
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
    // a single activation.
    int localId;
    int agentDir, nextAgentDir, prevAgentDir;
    int passTokensDir = -1;

    State agentState;
    SubPhase subPhase;
    LeaderElectionParticle* candidateParticle;

    // Variables for Segment Comparison:
    // comparingSegment is true if this agent is in the Segment Comparison
    // subphase and has generated and passed a segment lead token along its
    // front segment; it is false otherwise.
    // isCoveredCandidate is set to true when this candidate agent is covered by
    // another candidate and demotes itself; i.e., when it receives an active
    // token from its front segment. This indicates to a later final segment
    // clean token that the candidate that sent it covered another candidate in
    // its Segment Comparison subphase.
    // absorbedActiveToken is true if this agent has already absorbed an active
    // token, which tells it whether to absorb active tokens or pass them
    // backwards along the cycle in the Segment Comparison subphase.
    // generatedCleanToken is true if this agent was covered and generated a
    // cleaning token. This is needed because a particle can only write into the
    // memory of one neighbor per activation, but it needs to pass cleaning
    // tokens in both directions.
    bool comparingSegment = false;
    bool isCoveredCandidate = false;
    bool absorbedActiveToken = false;
    bool generatedCleanToken = false;

    // Variables for Coin Flipping and Candidacy Transferal:
    // gotAnnounceInCompare is set to true if this agent did not succeed in
    // covering another candidate in its Segment Comparison subphase but was
    // “saved” by a candidacy transferal; instead of demoting itself, it will
    // proceed with the next subphase.
    // gotAnnounceBeforeAck is set to true if this agent is in the Coin Flipping
    // subphase and receives a candidate announcement token before it receives a
    // candidate acknowledgement token; thus, instead of demoting itself, it
    // will proceed with Solitude Verification.
    // waitingForTransferAck is true if this agent generated a candidate
    // announcement token in the Coin Flipping subphase but has not yet received
    // an acknowledgement.
    bool gotAnnounceInCompare = false;
    bool gotAnnounceBeforeAck = false;
    bool waitingForTransferAck = false;

    // Variables for Solitude Verification:
    // createdLead is true if this agent generated a solitude active token and
    // passed it forward during Solitude Verification.
    // hasGeneratedTokens is true if this agent generated solitude vector tokens
    // using the solitude active token. This is used to avoid incorrectly mixing
    // tokens of different agents on the same particle in Solitude Verification.
    bool createdLead = false;
    bool hasGeneratedTokens = false;

    // Variables for Boundary Testing:
    // testingBorder is true if this agent is the sole candidate and has begun
    // the Boundary Testing subphase.
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
    // front and back segments gray
    void cleanSolitudeVerificationTokens();

    // Boundary Testing methods
    int addNextBorder(int currentSum) const;

    // Methods for passing, taking, and checking the ownership of tokens at the
    // agent level
    template <class TokenType>
    bool hasAgentToken(int agentDir) const;
    template <class TokenType>
    std::shared_ptr<TokenType> peekAgentToken(int agentDir) const;
    template <class TokenType>
    std::shared_ptr<TokenType> takeAgentToken(int agentDir);
    template <class TokenType>
    void passAgentToken(int agentDir, std::shared_ptr<TokenType> token);
    LeaderElectionAgent* nextAgent() const;
    LeaderElectionAgent* prevAgent() const;

    // Methods responsible for rendering the agents onto the simulator with
    // their colors changing based on the state and the subphase of the current
    // agent
    // Red --> Candidate agent in Segment Comparison Subphase
    // Yellow --> Candidate agent in Coin Flipping Subphase
    // Blue --> Candidate agent in Solitude Verification Subphase
    // Grey --> Demoted agent
    // Green --> Sole candidate
    void setStateColor();
    void setSubPhaseColor();

    // Methods responsible for painting the borders which will act as physical
    // representations of the cycle for leader election
    // Red --> Segment Comparison Phase
    // Yellow --> Coin Flipping Phase
    // Blue --> Solitude Verification Phase
    // Grey --> No phase (or, alternatively, active phase of Segment Comparison
    // phase
    void paintFrontSegment(const int color);
    void paintBackSegment(const int color);
  };

  protected:
   State state;
   unsigned int currentAgent;
   std::vector<LeaderElectionAgent*> agents;
   std::array<int, 18> borderColorLabels;
   std::array<int, 6> borderPointColorLabels;
};

class LeaderElectionSystem : public AmoebotSystem {
 public:
  // Constructs a system of LeaderElectionParticles with an optionally specified
  // size (#particles), and hole probability. holeProb in [0,1] controls how
  // "spread out" the system is; closer to 0 is more compressed, closer to 1 is
  // more expanded.
  LeaderElectionSystem(int numParticles = 100, double holeProb = 0.2);

  // Checks whether or not the system's run of the Leader Election algorithm has
  // terminated (all particles in state Finished or Leader).
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_LEADERELECTION_H_
