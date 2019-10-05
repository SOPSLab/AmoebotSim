#include <QtGlobal>
#include <QtDebug>

#include <set>
#include <vector>

#include "alg/improvedleaderelection.h"

//----------------------------BEGIN PARTICLE CODE----------------------------

LeaderElectionParticle::LeaderElectionParticle(const Node head,
                                               const int globalTailDir,
                                               const int orientation,
                                               AmoebotSystem& system,
                                               State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    currentAgent(0) {
  borderColorLabels.fill(-1);
  borderPointColorLabels.fill(-1);
}

void LeaderElectionParticle::activate() {
  if (state == State::Idle) {
    // Determine the number of neighbors of the current particle.
    // If there are no neighbors, then that means the particle is the only
    // one in the system and should declare itself as the leader.
    // If it is surrounded by 6 neighbors, then it cannot participate in
    // leader election.
    // Otherwise, the particle may participate in leader election and must
    // generate agents to do so.
    int numNbrs = getNumberOfNbrs();
    if (numNbrs == 0) {
      state = State::Leader;
      return;
    } else if (numNbrs == 6) {
      state = State::Finished;
    } else {
      int agentId = 0;
      for (int dir = 0; dir < 6; dir++) {
        if (!hasNbrAtLabel(dir) && hasNbrAtLabel((dir + 1) % 6)) {
          Q_ASSERT(agentId < 3);

          LeaderElectionAgent* agent = new LeaderElectionAgent();
          agent->candidateParticle = this;
          agent->localId = agentId + 1;
          agent->agentDir = dir;
          agent->nextAgentDir = getNextAgentDir(dir);
          agent->prevAgentDir = getPrevAgentDir(dir);
          agent->agentState = State::Candidate;
          agent->subPhase = LeaderElectionAgent::SubPhase::SegmentSetup;
          agent->setStateColor();

          agent->paintBackSegment(0x696969);
          agent->paintFrontSegment(0x696969);

          agents.push_back(agent);
          agentId++;
        }
      }
      state = State::Candidate;
      return;
    }
  } else if (state == State::Candidate) {
    agents.at(currentAgent)->activate();
    currentAgent = (currentAgent + 1) % agents.size();

    // The following is used by a particle in the candidate state to determine
    // whether or not to declare itself as the Leader or declare itself to be in
    // the Finished state depending on the state of its agents.
    bool allFinished = true;
    for (unsigned i = 0; i < agents.size(); i++) {
      LeaderElectionAgent* agent = agents.at(i);
      if (agent->agentState != State::Finished) {
        allFinished = false;
      }
      if (agent->agentState == State::Leader) {
        state = State::Leader;
        return;
      }
    }

    if (allFinished) {
      state = State::Finished;
    }
  } else if (state == State::Finished) {
    for (unsigned i = 0; i < agents.size(); i++) {
      LeaderElectionAgent* agent = agents.at(i);
      agent->cleanAllTokens();
      agent->paintBackSegment(-1);
      agent->paintFrontSegment(-1);
    }
  }

  return;
}

int LeaderElectionParticle::headMarkColor() const {
  if (state == State::Leader) {
    return 0x00ff00;
  }

  return -1;
}

int LeaderElectionParticle::headMarkDir() const {
  return -1;
}

int LeaderElectionParticle::tailMarkColor() const {
  return headMarkColor();
}

QString LeaderElectionParticle::inspectionText() const {
  QString text;
  text += "head: (" + QString::number(head.x) + ", " + QString::number(head.y) +
    ")\n";
  text += "orientation: " + QString::number(orientation) + "\n";
  text += "globalTailDir: " + QString::number(globalTailDir) + "\n";
  text += "state: ";
  text += [this](){
    switch(state) {
      case State::Idle:   return "idle";
      case State::Candidate:   return "candidate";
      case State::SoleCandidate: return "sole candidate";
      case State::Demoted:   return "demoted";
      case State::Leader: return "leader";
      case State::Finished: return "finished";
      default:            return "no state";
    }
  }();
  text += "\n";
  text += "number of agents: " + QString::number(agents.size()) + "\n";
  for (LeaderElectionAgent* agent : agents) {
    int nextAgentDir = agent->nextAgentDir;
    int prevAgentDir = agent->prevAgentDir;
    text += [agent](){
      switch(agent->agentState) {
        case State::Demoted: return "    demoted\n    ";
        case State::Candidate:
          switch(agent->subPhase) {
            case LeaderElectionParticle::LeaderElectionAgent::
            SubPhase::SegmentSetup: return "    segment setup\n    ";
            case LeaderElectionParticle::LeaderElectionAgent::
            SubPhase::IdentifierSetup: return "    identifier setup\n    ";
            case LeaderElectionParticle::LeaderElectionAgent::
            SubPhase::IdentifierComparison:
              return "    identifer comparison\n    ";
            case LeaderElectionParticle::LeaderElectionAgent::
            SubPhase::SolitudeVerification:
              return "    solitude verification\n    ";
          }
        case State::SoleCandidate: return "    sole candidate\n    ";
        default: return "invalid\n";
      }
    }();
    text += "    agent dir: " + QString::number(agent->agentDir) + "\n    ";
    text += "    next agent dir: " + QString::number(nextAgentDir) + "\n    ";
    text += "    prev agent dir: " + QString::number(prevAgentDir) + "\n    ";
    text += "    number of digit tokens: " +
        QString::number(agent->countAgentTokens<DigitToken>(nextAgentDir)) +
        "\n    ";
    text += "    number of delimiter tokens: " +
        QString::number(agent->countAgentTokens<DelimiterToken>(nextAgentDir)) +
        "\n    ";
    text += "    has generated reverse tokens: " +
        QString::number(agent->hasGeneratedReverseToken) + "\n    ";
    text += "    number of setup tokens: " +
        QString::number(agent->countAgentTokens<SetUpToken>(nextAgentDir) +
                        agent->countAgentTokens<SetUpToken>(prevAgentDir)) +
        "\n    ";
    text += "    id value: " + QString::number(agent->idValue) + "\n";
  }
  text += "has leader election tokens: " +
      QString::number(hasToken<LeaderElectionToken>()) + "\n";
  text += "has solitude active token: " +
      QString::number(hasToken<SolitudeActiveToken>()) + "\n";
  text += "has " + QString::number(countTokens<SolitudePositiveXToken>()) +
      " positive x tokens\n";
  text += "has " + QString::number(countTokens<SolitudeNegativeXToken>()) +
      " negative x tokens\n";
  text += "has " + QString::number(countTokens<SolitudePositiveYToken>()) +
      " positive y tokens\n";
  text += "has " + QString::number(countTokens<SolitudeNegativeYToken>()) +
      " negative y tokens\n";
  text += "\n";

  return text;
}

std::array<int, 18> LeaderElectionParticle::borderColors() const {
  return borderColorLabels;
}

std::array<int, 6> LeaderElectionParticle::borderPointColors() const {
  return borderPointColorLabels;
}

LeaderElectionParticle& LeaderElectionParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<LeaderElectionParticle>(label);
}

int LeaderElectionParticle::getNextAgentDir(const int agentDir) const {
  Q_ASSERT(!hasNbrAtLabel(agentDir));

  for (int dir = 1; dir < 6; dir++) {
    if (hasNbrAtLabel((agentDir - dir + 6) % 6)) {
      return (agentDir - dir + 6) % 6;
    }
  }

  Q_ASSERT(false);
  return -1;
}

int LeaderElectionParticle::getPrevAgentDir(const int agentDir) const {
  Q_ASSERT(!hasNbrAtLabel(agentDir));
  for (int dir = 1; dir < 6; dir++) {
    if (hasNbrAtLabel((agentDir + dir) % 6)) {
      return (agentDir + dir) % 6;
    }
  }

  Q_ASSERT(false);
  return -1;
}

int LeaderElectionParticle::getNumberOfNbrs() const {
  int count = 0;
  for (int dir = 0; dir < 6; dir++) {
    if (hasNbrAtLabel(dir)) {
      count++;
    }
  }
  return count;
}
//----------------------------END PARTICLE CODE----------------------------

//----------------------------BEGIN AGENT CODE----------------------------

LeaderElectionParticle::LeaderElectionAgent::LeaderElectionAgent() :
  localId(-1),
  agentDir(-1),
  nextAgentDir(-1),
  prevAgentDir(-1),
  agentState(State::Idle),
  candidateParticle(nullptr)
{}

void LeaderElectionParticle::LeaderElectionAgent::activate() {
  passTokensDir = randInt(0, 2);
  if (agentState == State::Candidate) {
    LeaderElectionAgent* next = nextAgent();
    LeaderElectionAgent* prev = prevAgent();

    if (subPhase == SubPhase::SegmentSetup) {
      bool coinFlip = randBool();
      if (coinFlip) {
        subPhase = SubPhase::IdentifierSetup;
      } else {
        agentState = State::Demoted;
      }
      setStateColor();
      return;
    }

    // Identifier Setup
    if (passTokensDir == 1 && hasAgentToken<SetUpToken>(prevAgentDir) &&
        prev != nullptr && prev->idValue != -1 &&
        !prev->hasGeneratedReverseToken) {
      takeAgentToken<SetUpToken>(prevAgentDir);
      passAgentToken<PrevIDPassToken>(prevAgentDir,
                                      std::make_shared<PrevIDPassToken>());
    }

    if (hasAgentToken<PrevIDPassToken>(nextAgentDir) &&
        !hasGeneratedReverseToken) {
      if (next != nullptr && (next->agentState == State::Candidate ||
                              next->demotedFromComparison)) {
        takeAgentToken<PrevIDPassToken>(nextAgentDir);
        candidateParticle->putToken(
              std::make_shared<DelimiterToken>(nextAgentDir, comparisonColor,
                                               idValue));
        hasGeneratedReverseToken = true;
        paintFrontSegment(comparisonColor);
        paintBackSegment(0x696969);
      } else if (passTokensDir == 0 && next != nullptr) {
        Q_ASSERT(next->idValue != -1);
        Q_ASSERT(!next->hasGeneratedReverseToken);

        std::shared_ptr<PrevIDPassToken> token =
            takeAgentToken<PrevIDPassToken>(nextAgentDir);
        candidateParticle->putToken(
              std::make_shared<DigitToken>(nextAgentDir, token->val));
        std::shared_ptr<NextIDPassToken> nextToken =
            std::make_shared<NextIDPassToken>(-1, idValue, comparisonColor);
        passAgentToken<NextIDPassToken>(nextAgentDir, nextToken);
        hasGeneratedReverseToken = true;
        paintFrontSegment(0x696969);
        paintBackSegment(0x696969);
      }
    }

    // Identifier Comparison
    if (hasAgentToken<DigitToken>(nextAgentDir)) {
      if (isActive && peekAgentToken<DigitToken>(nextAgentDir)->isActive) {
        isActive = false;
        peekAgentToken<DigitToken>(nextAgentDir)->isActive = false;
        demotedFromComparison = true;
      }
      if (canPassComparisonToken(false)) {
        peekAgentToken<DigitToken>(nextAgentDir)->isActive = true;
        passAgentToken<DigitToken>(prevAgentDir,
                                   takeAgentToken<DigitToken>(nextAgentDir));
      }
    }

    if (hasAgentToken<DelimiterToken>(nextAgentDir)) {
      if (isActive && peekAgentToken<DelimiterToken>(nextAgentDir)->isActive) {
        int tokenValue = peekAgentToken<DelimiterToken>(nextAgentDir)->value;
        int status = peekAgentToken<DelimiterToken>(nextAgentDir)->compare;
        compareStatus = idValue - tokenValue;
        isActive = false;
        peekAgentToken<DelimiterToken>(nextAgentDir)->isActive = false;
        if (compareStatus == -1 || (status == -1 && compareStatus == 0)) {
          demotedFromComparison = true;
        } else if (compareStatus == 0 && status == 0) {
          subPhase = SubPhase::SolitudeVerification;
          setStateColor();
        }
      }
      if (canPassComparisonToken(true)) {
        int color =
            peekAgentToken<DelimiterToken>(nextAgentDir)->comparisonColor;
        peekAgentToken<DelimiterToken>(nextAgentDir)->isActive = true;
        passAgentToken<DelimiterToken>(prevAgentDir,
                                       takeAgentToken<DelimiterToken>
                                       (nextAgentDir));
        if (prev->agentState == State::Candidate) {
          prev->paintFrontSegment(color);
        } else {
          prev->paintFrontSegment(color);
          prev->paintBackSegment(color);
        }
        paintFrontSegment(0x696969);
        isActive = true;
      }
    }

    // Solitude Verification
    // Here we check whether or not the SolitudeActiveToken has a set local_id
    // to avoid the a possibility that the current agent might incorrectly
    // assume that the SolitudeActiveToken owned by the particle is meant for
    // the agent to pass back (and set the local_id value for); for example, in
    // the case where a candidate agent's nextAgentDir and prevAgentDir are the
    // same.
    if (passTokensDir == 1 &&
        hasAgentToken<SolitudeActiveToken>(prevAgentDir) &&
        peekAgentToken<SolitudeActiveToken>(prevAgentDir)->local_id == -1) {
      peekAgentToken<SolitudeActiveToken>(prevAgentDir)->local_id = localId;
      passAgentToken<SolitudeActiveToken>
          (prevAgentDir, takeAgentToken<SolitudeActiveToken>(prevAgentDir));
      paintBackSegment(0x696969);
    }

    if (hasAgentToken<SolitudeNegativeXToken>(nextAgentDir)) {
      peekAgentToken<SolitudeNegativeXToken>(nextAgentDir)->isSettled = true;
    }

    if (hasAgentToken<SolitudePositiveXToken>(nextAgentDir)) {
      peekAgentToken<SolitudePositiveXToken>(nextAgentDir)->isSettled = true;
    }

    if (hasAgentToken<SolitudeNegativeYToken>(nextAgentDir)) {
      peekAgentToken<SolitudeNegativeYToken>(nextAgentDir)->isSettled = true;
    }

    if (hasAgentToken<SolitudePositiveYToken>(nextAgentDir)) {
      peekAgentToken<SolitudePositiveYToken>(nextAgentDir)->isSettled = true;
    }

    if (subPhase == SubPhase::IdentifierSetup) {
      if (!hasGeneratedSetupToken && passTokensDir == 0) {
        passAgentToken<SetUpToken>(nextAgentDir,
                                   std::make_shared<SetUpToken>
                                   (comparisonColor));
        idValue = randInt(0,2);
        hasGeneratedSetupToken = true;
        paintFrontSegment(0xffa500);
        return;
      } else if (hasGeneratedReverseToken) {
        subPhase = SubPhase::IdentifierComparison;
        setStateColor();
        return;
      }
    } else if (subPhase == SubPhase::IdentifierComparison) {
      if (demotedFromComparison) {
        agentState = State::Demoted;
        setStateColor();
        return;
      }
    } else if (subPhase == SubPhase::SolitudeVerification) {
      if (!createdLead && passTokensDir == 0) {
        passAgentToken<SolitudeActiveToken>
            (nextAgentDir, std::make_shared<SolitudeActiveToken>());
        candidateParticle->putToken
            (std::make_shared<SolitudePositiveXToken>(nextAgentDir, true));
        paintFrontSegment(0x00bfff);
        createdLead = true;
        hasGeneratedTokens = true;
      } else if (hasAgentToken<SolitudeActiveToken>(nextAgentDir) &&
                 peekAgentToken<SolitudeActiveToken>
                 (nextAgentDir)->local_id != -1) {
        int checkX = checkSolitudeXTokens();
        int checkY = checkSolitudeYTokens();
        bool isSole =
            peekAgentToken<SolitudeActiveToken>(nextAgentDir)->isSoleCandidate;
        int id = peekAgentToken<SolitudeActiveToken>(nextAgentDir)->local_id;
        if (isSole && localId == id && checkX == 2 && checkY == 2) {
          agentState = State::SoleCandidate;
        } else if (checkX == 1 || checkY == 1) {
          // It should never reach this state since all solitude vector tokens
          // that reach a candidate agent are settled
          Q_ASSERT(false);
          return;
        } else if (!demotedFromComparison) {
          subPhase = SubPhase::IdentifierComparison;
        } else {
          agentState = State::Demoted;
        }
        takeAgentToken<SolitudeActiveToken>(nextAgentDir);
        createdLead = false;
        cleanSolitudeVerificationTokens();
        setStateColor();
        return;
      }
    }
  } else if (agentState == State::Demoted) {
    LeaderElectionAgent* next = nextAgent();
    LeaderElectionAgent* prev = prevAgent();

    if (passTokensDir == 0 && hasAgentToken<BorderTestToken>(prevAgentDir)) {
      std::shared_ptr<BorderTestToken> token =
          takeAgentToken<BorderTestToken>(prevAgentDir);
      token->borderSum = addNextBorder(token->borderSum);
      passAgentToken<BorderTestToken>(nextAgentDir, token);
      cleanAllTokens();
      paintBackSegment(-1);
      paintFrontSegment(-1);
      agentState = State::Finished;
      setStateColor();
    }

    // Identifier Setup
    if (hasAgentToken<SetUpToken>(prevAgentDir)) {
      if (demotedFromComparison && passTokensDir == 1 && prev != nullptr) {
        Q_ASSERT(prev->idValue != -1);
        Q_ASSERT(!prev->hasGeneratedReverseToken);

        takeAgentToken<SetUpToken>(prevAgentDir);
        passAgentToken<PrevIDPassToken>(prevAgentDir,
                                        std::make_shared<PrevIDPassToken>());
      } else if (!demotedFromComparison && passTokensDir == 0) {
        Q_ASSERT(idValue == -1);

        idValue = randInt(0, 2);
        passAgentToken<SetUpToken>(nextAgentDir,
                                   takeAgentToken<SetUpToken>(prevAgentDir));
        paintFrontSegment(0xffa500);
        paintBackSegment(0xffa500);
      }
    }

    if (hasAgentToken<PrevIDPassToken>(nextAgentDir) &&
        !hasGeneratedReverseToken) {
      if (prev != nullptr && prev->hasGeneratedReverseToken) {
        if (next != nullptr && next->hasGeneratedReverseToken) {
          takeAgentToken<PrevIDPassToken>(nextAgentDir);
          candidateParticle->putToken(
                std::make_shared<DigitToken>(nextAgentDir, idValue));
          hasGeneratedReverseToken = true;
          paintFrontSegment(0x696969);
          paintBackSegment(0x696969);
        } else if (next != nullptr && passTokensDir == 0) {
          std::shared_ptr<PrevIDPassToken> token =
              takeAgentToken<PrevIDPassToken>(nextAgentDir);
          candidateParticle->putToken(
                std::make_shared<DigitToken>(nextAgentDir, token->val));
          std::shared_ptr<NextIDPassToken> nextToken =
              std::make_shared<NextIDPassToken>();
          nextToken->val = idValue;
          hasGeneratedReverseToken = true;
          passAgentToken<NextIDPassToken>(nextAgentDir, nextToken);
          paintFrontSegment(0x696969);
          paintBackSegment(0x696969);
        }
      } else if (passTokensDir == 1 && prev != nullptr) {
        std::shared_ptr<PrevIDPassToken> token =
            takeAgentToken<PrevIDPassToken>(nextAgentDir);
        if (next != nullptr && (next->agentState == State::Candidate ||
                                next->hasGeneratedReverseToken ||
                                next->demotedFromComparison)) {
          token->val = idValue;
        }
        passAgentToken<PrevIDPassToken>(prevAgentDir, token);
      }
    }

    if (hasAgentToken<NextIDPassToken>(prevAgentDir) &&
        !hasGeneratedReverseToken) {
      if (next != nullptr && prev != nullptr && passTokensDir == 1 &&
          (next->agentState == State::Candidate ||
           next->hasGeneratedReverseToken || next->demotedFromComparison)) {
        std::shared_ptr<NextIDPassToken> token =
            takeAgentToken<NextIDPassToken>(prevAgentDir);
        if (next->agentState == State::Candidate ||
            next->demotedFromComparison) {
          candidateParticle->putToken(
                std::make_shared<DelimiterToken>(nextAgentDir,
                                                 token->comparisonColor,
                                                 token->val));
          paintFrontSegment(token->comparisonColor);
          paintBackSegment(0x696969);
        } else {
          candidateParticle->putToken(
                std::make_shared<DigitToken>(nextAgentDir, token->val));
          paintFrontSegment(0x696969);
          paintBackSegment(0x696969);
        }
        hasGeneratedReverseToken = true;
        if (!prev->hasGeneratedReverseToken) {
          passAgentToken<PrevIDPassToken>(prevAgentDir,
                                          std::make_shared<PrevIDPassToken>());
        }
      } else if (next != nullptr && passTokensDir == 0 &&
                 !(next->agentState == State::Candidate ||
                   next->hasGeneratedReverseToken ||
                   next->demotedFromComparison)) {
        passAgentToken<NextIDPassToken>(nextAgentDir,
                                        takeAgentToken<NextIDPassToken>
                                        (prevAgentDir));
      }
    }

    // Identifier Comparison
    if (hasAgentToken<DigitToken>(nextAgentDir)) {
      if (demotedFromComparison) {
        if (canPassComparisonToken(false)) {
          peekAgentToken<DigitToken>(nextAgentDir)->isActive = true;
          passAgentToken<DigitToken>(prevAgentDir,
                                     takeAgentToken<DigitToken>(nextAgentDir));
        }
      } else {
        if (isActive && peekAgentToken<DigitToken>(nextAgentDir)->isActive) {
          isActive = false;
          peekAgentToken<DigitToken>(nextAgentDir)->isActive = false;
          int tokenValue = peekAgentToken<DigitToken>(nextAgentDir)->value;
          compareStatus = idValue - tokenValue;
        }
        if (canPassComparisonToken(false)) {
          passAgentToken<DigitToken>(prevAgentDir,
                                     takeAgentToken<DigitToken>(nextAgentDir));
        }
      }
    }

    if (hasAgentToken<DelimiterToken>(nextAgentDir)) {
      if (demotedFromComparison) {
        if (canPassComparisonToken(true)) {
          int color =
              peekAgentToken<DelimiterToken>(nextAgentDir)->comparisonColor;
          peekAgentToken<DelimiterToken>(nextAgentDir)->isActive = true;
          passAgentToken<DelimiterToken>(prevAgentDir,
                                        takeAgentToken<DelimiterToken>
                                        (nextAgentDir));
          prev->paintFrontSegment(color);
          prev->paintBackSegment(color);
          paintFrontSegment(0x696969);
          paintBackSegment(0x696969);
        }
      } else {
        if (isActive &&
            peekAgentToken<DelimiterToken>(nextAgentDir)->isActive) {
          isActive = false;
          peekAgentToken<DelimiterToken>(nextAgentDir)->isActive = false;
        }
        peekAgentToken<DelimiterToken>(nextAgentDir)->compare = compareStatus;
        if (canPassComparisonToken(true)) {
          int color =
              peekAgentToken<DelimiterToken>(nextAgentDir)->comparisonColor;
          isActive = true;
          passAgentToken<DelimiterToken>(prevAgentDir,
                                         takeAgentToken<DelimiterToken>
                                         (nextAgentDir));
          compareStatus = 0;
          prev->paintFrontSegment(color);
          prev->paintBackSegment(color);
          paintFrontSegment(0x696969);
          paintBackSegment(0x696969);
        }
      }
    }

    // Solitude Verification Tokens
    if (passTokensDir == 0 &&
        hasAgentToken<SolitudeActiveToken>(prevAgentDir) &&
        peekAgentToken<SolitudeActiveToken>(prevAgentDir)->local_id == -1 &&
        !hasGeneratedTokens) {
      std::shared_ptr<SolitudeActiveToken> token =
          takeAgentToken<SolitudeActiveToken>(prevAgentDir);
      std::pair<int, int> generatedPair = augmentDirVector(token->vector);
      generateSolitudeVectorTokens(generatedPair);
      token->vector = generatedPair;
      paintBackSegment(0x00bfff);
      paintFrontSegment(0x00bfff);
      passAgentToken<SolitudeActiveToken>(nextAgentDir, token);
      hasGeneratedTokens = true;
    } else if (passTokensDir == 1 &&
               hasAgentToken<SolitudeActiveToken>(nextAgentDir) &&
               peekAgentToken<SolitudeActiveToken>
               (nextAgentDir)->local_id != -1 &&
               hasGeneratedTokens) {
      int checkX = checkSolitudeXTokens();
      int checkY = checkSolitudeYTokens();
      if ((checkX == 2 && checkY == 2) ||
          !peekAgentToken<SolitudeActiveToken>(nextAgentDir)->isSoleCandidate) {
        passAgentToken<SolitudeActiveToken>
            (prevAgentDir, takeAgentToken<SolitudeActiveToken>(nextAgentDir));
        cleanSolitudeVerificationTokens();
      } else if (checkX == 0 || checkY == 0) {
        std::shared_ptr<SolitudeActiveToken> token =
            takeAgentToken<SolitudeActiveToken>(nextAgentDir);
        token->isSoleCandidate = false;
        passAgentToken<SolitudeActiveToken>(prevAgentDir, token);
        cleanSolitudeVerificationTokens();
      }
    }

    if (hasAgentToken<SolitudeNegativeXToken>(nextAgentDir) &&
        !peekAgentToken<SolitudeNegativeXToken>(nextAgentDir)->isSettled &&
        hasGeneratedTokens) {
      if (passTokensDir == 1 && prev != nullptr &&
          !prev->hasAgentToken<SolitudeNegativeXToken>(prev->nextAgentDir)) {
        passAgentToken<SolitudeNegativeXToken>
            (prevAgentDir,
             takeAgentToken<SolitudeNegativeXToken>(nextAgentDir));
      } else if (prev != nullptr &&
                 prev->hasAgentToken<SolitudeNegativeXToken>
                 (prev->nextAgentDir) &&
                 prev->peekAgentToken<SolitudeNegativeXToken>
                 (prev->nextAgentDir)->isSettled) {
        peekAgentToken<SolitudeNegativeXToken>(nextAgentDir)->isSettled = true;
      }
    }

    if (hasAgentToken<SolitudePositiveXToken>(nextAgentDir) &&
        !peekAgentToken<SolitudePositiveXToken>(nextAgentDir)->isSettled &&
        hasGeneratedTokens) {
      if (passTokensDir == 1 && prev != nullptr &&
          !prev->hasAgentToken<SolitudePositiveXToken>(prev->nextAgentDir)) {
        passAgentToken<SolitudePositiveXToken>
            (prevAgentDir,
             takeAgentToken<SolitudePositiveXToken>(nextAgentDir));
      } else if (prev != nullptr &&
                 prev->hasAgentToken<SolitudePositiveXToken>
                 (prev->nextAgentDir) &&
                 prev->peekAgentToken<SolitudePositiveXToken>
                 (prev->nextAgentDir)->isSettled) {
        peekAgentToken<SolitudePositiveXToken>(nextAgentDir)->isSettled = true;
      }
    }

    if (hasAgentToken<SolitudeNegativeYToken>(nextAgentDir) &&
        !peekAgentToken<SolitudeNegativeYToken>(nextAgentDir)->isSettled &&
        hasGeneratedTokens) {
      if (passTokensDir == 1 && prev != nullptr &&
          !prev->hasAgentToken<SolitudeNegativeYToken>(prev->nextAgentDir)) {
        passAgentToken<SolitudeNegativeYToken>
            (prevAgentDir,
             takeAgentToken<SolitudeNegativeYToken>(nextAgentDir));
      } else if (prev != nullptr &&
                 prev->hasAgentToken<SolitudeNegativeYToken>
                 (prev->nextAgentDir) &&
                 prev->peekAgentToken<SolitudeNegativeYToken>
                 (prev->nextAgentDir)->isSettled) {
        peekAgentToken<SolitudeNegativeYToken>(nextAgentDir)->isSettled = true;
      }
    }

    if (hasAgentToken<SolitudePositiveYToken>(nextAgentDir) &&
        !peekAgentToken<SolitudePositiveYToken>(nextAgentDir)->isSettled &&
        hasGeneratedTokens) {
      if (passTokensDir == 1 && prev != nullptr &&
          !prev->hasAgentToken<SolitudePositiveYToken>(prev->nextAgentDir)) {
        passAgentToken<SolitudePositiveYToken>
            (prevAgentDir,
             takeAgentToken<SolitudePositiveYToken>(nextAgentDir));
      } else if (prev != nullptr &&
                 prev->hasAgentToken<SolitudePositiveYToken>
                 (prev->nextAgentDir) &&
                 prev->peekAgentToken<SolitudePositiveYToken>
                 (prev->nextAgentDir)->isSettled) {
        peekAgentToken<SolitudePositiveYToken>(nextAgentDir)->isSettled = true;
      }
    }

  } else if (agentState == State::SoleCandidate) {
    if (!testingBorder) {
      std::shared_ptr<BorderTestToken> token =
          std::make_shared<BorderTestToken>(prevAgentDir, addNextBorder(0));
      passAgentToken<BorderTestToken>(nextAgentDir, token);
      paintFrontSegment(-1);
      testingBorder = true;
    } else if (hasAgentToken<BorderTestToken>(prevAgentDir) &&
               peekAgentToken<BorderTestToken>(prevAgentDir)->borderSum != -1) {
      int borderSum = takeAgentToken<BorderTestToken>(prevAgentDir)->borderSum;
      paintBackSegment(-1);
      if (borderSum == 1) {
        agentState = State::Leader;
      } else if (borderSum == 4) {
        agentState = State::Finished;
      } else {
        Q_ASSERT(false);
      }
      setStateColor();
      testingBorder = false;
      return;
    }
  } else if (agentState == State::Finished) {
    cleanAllTokens();
    paintBackSegment(-1);
    paintFrontSegment(-1);
  }
}

void LeaderElectionParticle::LeaderElectionAgent::cleanAllTokens() {
  cleanSolitudeVerificationTokens();
  while (hasAgentToken<DigitToken>(nextAgentDir)) {
    takeAgentToken<DigitToken>(nextAgentDir);
  }
  if (hasAgentToken<DelimiterToken>(nextAgentDir)) {
    takeAgentToken<DelimiterToken>(nextAgentDir);
  }
}

bool LeaderElectionParticle::LeaderElectionAgent::
canPassComparisonToken(bool isDelimiter) const {
  LeaderElectionAgent* prev = prevAgent();
  if (prev == nullptr || !prev->hasGeneratedReverseToken ||
      passTokensDir != 1) {
    return false;
  }
  int prevNextAgentDir = prev->nextAgentDir;
  if (isDelimiter) {
    if (!prev->hasAgentToken<DigitToken>(prevNextAgentDir) &&
        !prev->hasAgentToken<DelimiterToken>(prevNextAgentDir)) {
      return true;
    }
  } else {
    int prevCountDigit = prev->countAgentTokens<DigitToken>(prevNextAgentDir);
    if (prevCountDigit < 2 &&
        !prev->hasAgentToken<DelimiterToken>(prevNextAgentDir)) {
      return true;
    }
  }
  return false;
}

std::pair<int, int> LeaderElectionParticle::LeaderElectionAgent::
augmentDirVector(std::pair<int, int> vector) {
  unsigned int offset = (nextAgentDir - ((prevAgentDir + 3) % 6) + 6) % 6;
  const std::array<std::pair<int, int>, 6> vectors =
  { std::make_pair(1, 0), std::make_pair(0, 1), std::make_pair(-1, 1),
    std::make_pair(-1, 0), std::make_pair(0, -1), std::make_pair(1, -1) };

  for (unsigned i = 0; i < vectors.size(); ++i) {
    if (vector == vectors.at(i)) {
      return vectors.at((i + offset) % 6);
    }
  }

  Q_ASSERT(false);
  return std::make_pair(0, 0);
}

void LeaderElectionParticle::LeaderElectionAgent::
generateSolitudeVectorTokens(std::pair<int, int> vector) {
  // We initialize the solitude vector tokens with an origin direction
  // of nextAgentDir because this is the only direction from which these
  // tokens can come from, so it does not matter whether or not these tokens
  // were generated by the agent or if they were passed to the agent.
  switch(vector.first) {
    case -1:
      candidateParticle->putToken
          (std::make_shared<SolitudeNegativeXToken>(nextAgentDir, false));
      break;
    case 0:
      break;
    case 1:
      candidateParticle->putToken
          (std::make_shared<SolitudePositiveXToken>(nextAgentDir, false));
      break;
    default:
      Q_ASSERT(false);
      break;
  }
  switch(vector.second) {
    case -1:
      candidateParticle->putToken
          (std::make_shared<SolitudeNegativeYToken>(nextAgentDir, false));
      break;
    case 0:
      break;
    case 1:
      candidateParticle->putToken
          (std::make_shared<SolitudePositiveYToken>(nextAgentDir, false));
      break;
    default:
      Q_ASSERT(false);
      break;
  }
}

int LeaderElectionParticle::LeaderElectionAgent::checkSolitudeXTokens() const {
  if (hasAgentToken<SolitudePositiveXToken>(nextAgentDir) &&
      hasAgentToken<SolitudeNegativeXToken>(nextAgentDir)) {
    if (peekAgentToken<SolitudePositiveXToken>(nextAgentDir)->isSettled &&
        peekAgentToken<SolitudeNegativeXToken>(nextAgentDir)->isSettled) {
      return 2;
    } else {
      return 1;
    }
  } else if (hasAgentToken<SolitudePositiveXToken>(nextAgentDir)) {
    if (peekAgentToken<SolitudePositiveXToken>(nextAgentDir)->isSettled) {
      return 0;
    } else {
      return 1;
    }
  } else if (hasAgentToken<SolitudeNegativeXToken>(nextAgentDir)) {
    if (peekAgentToken<SolitudeNegativeXToken>(nextAgentDir)->isSettled) {
      return 0;
    } else {
      return 1;
    }
  } else {
    return 2;
  }
}

int LeaderElectionParticle::LeaderElectionAgent::checkSolitudeYTokens() const {
  if (hasAgentToken<SolitudePositiveYToken>(nextAgentDir) &&
      hasAgentToken<SolitudeNegativeYToken>(nextAgentDir)) {
    if (peekAgentToken<SolitudePositiveYToken>(nextAgentDir)->isSettled &&
        peekAgentToken<SolitudeNegativeYToken>(nextAgentDir)->isSettled) {
      return 2;
    } else {
      return 1;
    }
  } else if (hasAgentToken<SolitudePositiveYToken>(nextAgentDir)) {
    if (peekAgentToken<SolitudePositiveYToken>(nextAgentDir)->isSettled) {
      return 0;
    } else {
      return 1;
    }
  } else if (hasAgentToken<SolitudeNegativeYToken>(nextAgentDir)) {
    if (peekAgentToken<SolitudeNegativeYToken>(nextAgentDir)->isSettled) {
      return 0;
    } else {
      return 1;
    }
  } else {
    return 2;
  }
}

void LeaderElectionParticle::LeaderElectionAgent::
cleanSolitudeVerificationTokens() {
  if (hasAgentToken<SolitudePositiveXToken>(nextAgentDir)) {
    takeAgentToken<SolitudePositiveXToken>(nextAgentDir);
  }
  if (hasAgentToken<SolitudePositiveYToken>(nextAgentDir)) {
    takeAgentToken<SolitudePositiveYToken>(nextAgentDir);
  }
  if (hasAgentToken<SolitudeNegativeXToken>(nextAgentDir)) {
    takeAgentToken<SolitudeNegativeXToken>(nextAgentDir);
  }
  if (hasAgentToken<SolitudeNegativeYToken>(nextAgentDir)) {
    takeAgentToken<SolitudeNegativeYToken>(nextAgentDir);
  }
  paintFrontSegment(0x696969);
  paintBackSegment(0x696969);
  hasGeneratedTokens = false;
}

int LeaderElectionParticle::LeaderElectionAgent::
addNextBorder(int currentSum) const {
  // adjust offset in modulo 6 to be compatible with modulo 5 computations
  int offsetMod6 = (prevAgentDir + 3) % 6 - nextAgentDir;
  if(4 <= offsetMod6 && offsetMod6 <= 5) {
      offsetMod6 -= 6;
  } else if(-5 <= offsetMod6 && offsetMod6 <= -3) {
      offsetMod6 += 6;
  }

  return (currentSum + offsetMod6 + 5) % 5;
}

template <class TokenType>
bool LeaderElectionParticle::LeaderElectionAgent::
hasAgentToken(int agentDir) const{
    auto prop = [agentDir](const std::shared_ptr<TokenType> token) {
      return token->origin == agentDir;
    };
    return candidateParticle->hasToken<TokenType>(prop);
}

template <class TokenType>
std::shared_ptr<TokenType>
LeaderElectionParticle::LeaderElectionAgent::
peekAgentToken(int agentDir) const {
  auto prop = [agentDir](const std::shared_ptr<TokenType> token) {
    return token->origin == agentDir;
  };
  return candidateParticle->peekAtToken<TokenType>(prop);
}

template <class TokenType>
std::shared_ptr<TokenType>
LeaderElectionParticle::LeaderElectionAgent::takeAgentToken(int agentDir) {
  auto prop = [agentDir](const std::shared_ptr<TokenType> token) {
    return token->origin == agentDir;
  };
  return candidateParticle->takeToken<TokenType>(prop);
}

template <class TokenType>
void LeaderElectionParticle::LeaderElectionAgent::
passAgentToken(int agentDir, std::shared_ptr<TokenType> token) {
  LeaderElectionParticle* nbr = &candidateParticle->nbrAtLabel(agentDir);
  int origin = -1;
  for (int i = 0; i < 6; i++) {
    if (nbr->hasNbrAtLabel(i) && &nbr->nbrAtLabel(i) == candidateParticle) {
      origin = i;
      break;
    }
  }
  Q_ASSERT(origin != -1);
  token->origin = origin;
  nbr->putToken(token);
}

template <class TokenType>
int LeaderElectionParticle::LeaderElectionAgent::
countAgentTokens(int agentDir) const {
  auto prop = [agentDir](const std::shared_ptr<TokenType> token) {
    return token->origin == agentDir;
  };
  return candidateParticle->countTokens<TokenType>(prop);
}

LeaderElectionParticle::LeaderElectionAgent*
LeaderElectionParticle::LeaderElectionAgent::nextAgent() const {
  LeaderElectionParticle* nextNbr =
      &candidateParticle->nbrAtLabel(nextAgentDir);
  int originLabel = -1;
  for (int i = 0; i < 6; i++) {
    if (nextNbr->hasNbrAtLabel(i) &&
        &nextNbr->nbrAtLabel(i) == candidateParticle) {
      originLabel = i;
      break;
    }
  }
  Q_ASSERT(originLabel != -1);
  for (LeaderElectionAgent* agent : nextNbr->agents) {
    if (agent->prevAgentDir == originLabel) {
      return agent;
    }
  }
  Q_ASSERT(nextNbr->agents.size() == 0);
  return nullptr;
}

LeaderElectionParticle::LeaderElectionAgent*
LeaderElectionParticle::LeaderElectionAgent::prevAgent() const {
  LeaderElectionParticle* prevNbr =
      &candidateParticle->nbrAtLabel(prevAgentDir);
  int originLabel = -1;
  for (int i = 0; i < 6; i++) {
    if (prevNbr->hasNbrAtLabel(i) &&
        &prevNbr->nbrAtLabel(i) == candidateParticle) {
      originLabel = i;
      break;
    }
  }
  Q_ASSERT(originLabel != -1);
  for (LeaderElectionAgent* agent : prevNbr->agents) {
    if (agent->nextAgentDir == originLabel) {
      return agent;
    }
  }
  Q_ASSERT(prevNbr->agents.size() == 0);
  return nullptr;
}

void LeaderElectionParticle::LeaderElectionAgent::setStateColor() {
  int globalizedDir = candidateParticle->localToGlobalDir(agentDir);
  switch (agentState) {
    case State::Candidate:
      setSubPhaseColor();
      break;
    case State::Demoted:
      candidateParticle->borderPointColorLabels.at(globalizedDir) = 0x696969;
      break;
    case State::SoleCandidate:
      candidateParticle->borderPointColorLabels.at(globalizedDir) = 0x00ff00;
      break;
    case State::Leader:
      candidateParticle->borderPointColorLabels.at(globalizedDir) = 0x00ff00;
      break;
    case State::Finished:
      candidateParticle->borderPointColorLabels.at(globalizedDir) = -1;
      break;
    default:
      break;
  }
}

void LeaderElectionParticle::LeaderElectionAgent::setSubPhaseColor() {
  int globalizedDir = candidateParticle->localToGlobalDir(agentDir);
  switch (subPhase) {
    case SubPhase::SegmentSetup:
      candidateParticle->borderPointColorLabels.at(globalizedDir) = 0xff0000;
      break;
    case SubPhase::IdentifierSetup:
      candidateParticle->borderPointColorLabels.at(globalizedDir) = 0xffa500;
      break;
    case SubPhase::IdentifierComparison:
      candidateParticle->borderPointColorLabels.at(globalizedDir) =
          comparisonColor;
      break;
    case SubPhase::SolitudeVerification:
      candidateParticle->borderPointColorLabels.at(globalizedDir) = 0x00bfff;
      break;
    default:
      Q_ASSERT(false);
      break;
  }
}

void LeaderElectionParticle::LeaderElectionAgent::paintFrontSegment(
    const int color) {
  // Must use localToGlobalDir method to reconcile the difference between the
  // local orientation of the particle and the global orientation used by
  // drawing
  int tempDir = candidateParticle->localToGlobalDir(agentDir);
  int tempNextDir = candidateParticle->localToGlobalDir(nextAgentDir);
  while (tempDir != (tempNextDir + 1) % 6) {
      if ((tempDir + 5) % 6 != tempNextDir) {
          candidateParticle->borderColorLabels.at((3 * tempDir + 17) % 18) =
              color;
      }
      tempDir = (tempDir + 5) % 6;
  }
}

void LeaderElectionParticle::LeaderElectionAgent::paintBackSegment(
    const int color) {
  // Must use localToGlobalDir method to reconcile the difference between the
  // local orientation of the particle and the global orientation used by
  // drawing
  candidateParticle->borderColorLabels.at(
        3 * candidateParticle->localToGlobalDir(agentDir) + 1) = color;
}

//----------------------------END AGENT CODE----------------------------

//----------------------------BEGIN SYSTEM CODE----------------------------

ImprovedLeaderElectionSystem::ImprovedLeaderElectionSystem(int numParticles,
                                                   double holeProb) {
  Q_ASSERT(numParticles > 0);
  Q_ASSERT(0 <= holeProb && holeProb <= 1);

  // Insert the seed at (0,0).
  insert(new LeaderElectionParticle(Node(0, 0), -1, randDir(), *this,
                                    LeaderElectionParticle::State::Idle));
  std::set<Node> occupied;
  occupied.insert(Node(0, 0));

  std::set<Node> candidates;
  for (int i = 0; i < 6; ++i) {
    candidates.insert(Node(0, 0).nodeInDir(i));
  }

  // Add inactive particles.
  int numNonStaticParticles = 0;
  while (numNonStaticParticles < numParticles && !candidates.empty()) {
    // Pick random candidate.
    int randIndex = randInt(0, candidates.size());
    Node randomCandidate;
    for (auto it = candidates.begin(); it != candidates.end(); ++it) {
      if (randIndex == 0) {
        randomCandidate = *it;
        candidates.erase(it);
        break;
      } else {
        randIndex--;
      }
    }

    occupied.insert(randomCandidate);

    // Add this candidate as a particle if not a hole.
    if (randBool(1.0 - holeProb)) {
      insert(new LeaderElectionParticle(randomCandidate, -1, randDir(), *this,
                                        LeaderElectionParticle::State::Idle));
      ++numNonStaticParticles;

      // Add new candidates.
      for (int i = 0; i < 6; ++i) {
        auto neighbor = randomCandidate.nodeInDir(i);
        if (occupied.find(neighbor) == occupied.end()) {
          candidates.insert(neighbor);
        }
      }
    }
  }
}

bool ImprovedLeaderElectionSystem::hasTerminated() const {
  #ifdef QT_DEBUG
    if (!isConnected(particles)) {
      return true;
    }
  #endif

  for (auto p : particles) {
    auto hp = dynamic_cast<LeaderElectionParticle*>(p);
    if (hp->state != LeaderElectionParticle::State::Leader &&
        hp->state != LeaderElectionParticle::State::Finished) {
      return false;
    }
  }

  return true;
}
