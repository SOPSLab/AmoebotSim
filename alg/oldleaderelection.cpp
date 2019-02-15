#include <QtGlobal>
#include <QtDebug>

#include <set>
#include <vector>

#include "alg/oldleaderelection.h"

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
  if (state == State::Demoted && hasToken<LeaderElectionToken>()) {
    agents.at(currentAgent)->activate();
    currentAgent = (currentAgent + 1) % agents.size();
  } else if (state == State::Idle) {
    int numNbrs = getNumberOfNbrs();
    if (numNbrs == 0) {
      state = State::Leader;
      return;
    } else if (numNbrs == 6) {
      state = State::Finished;
    } else {
      for (int dir = 0; dir < 6; dir++) {
        int agentId = 0;
        if (!hasNbrAtLabel(dir) && hasNbrAtLabel((dir + 1) % 6)) {
          Q_ASSERT(agentId < 3);

          LeaderElectionAgent* agent = new LeaderElectionAgent();
          agent->candidateParticle = this;
          agent->localId = agentId + 1;
          agent->agentDir = dir;
          agent->nextAgentDir = getNextAgentDir(dir);
          agent->prevAgentDir = getPrevAgentDir(dir);
          agent->agentState = State::Candidate;
          agent->subPhase = LeaderElectionAgent::SubPhase::SegmentComparison;
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
    // Check whether or not all of the particle's agents have been demoted
    // If this is the case, then the particle is considered "Finished"
    bool isDemoted = true;

    for (unsigned i = 0; i < agents.size(); i++) {
      if (agents.at(i)->agentState != State::Demoted) {
        isDemoted = false;
        break;
      }
    }

    if (isDemoted) {
      state = State::Demoted;
      return;
    }

    agents.at(currentAgent)->activate();
    currentAgent = (currentAgent + 1) % agents.size();
  }

  return;
}

int LeaderElectionParticle::headMarkColor() const {
  if (state == State::Leader) {
    return 0xff0000;
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

int LeaderElectionParticle::labelOfFirstNbrInState(
    std::initializer_list<State> states, int startLabel) const {
  auto prop = [&](const LeaderElectionParticle& p) {
    for (auto state : states) {
      if (p.state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<LeaderElectionParticle>(prop, startLabel);
}

bool LeaderElectionParticle::hasNbrInState(std::initializer_list<State> states)
    const {
  return labelOfFirstNbrInState(states) != -1;
}

bool LeaderElectionParticle::canFinish() const {
  return false;
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
  if (agentState == State::Candidate) {
    if (hasAgentToken<ActiveSegmentCleanToken>(nextAgentDir)) {
      activeClean(nextAgentDir);
    }
    if (hasAgentToken<PassiveSegmentCleanToken>(prevAgentDir)) {
      passiveClean(prevAgentDir);
      paintBackSegment(0x696969);
    }
    if (hasAgentToken<SegmentLeadToken>(prevAgentDir)) {
      takeAgentToken<SegmentLeadToken>(prevAgentDir);
      passAgentToken<PassiveSegmentToken>(prevAgentDir, true);
      paintBackSegment(0x696969);
    }
    if (hasAgentToken<ActiveSegmentToken>(nextAgentDir)) {
      if (!absorbedActiveToken) {
        if (takeAgentToken<ActiveSegmentToken>(nextAgentDir)->isFinal) {
          passAgentToken<FinalSegmentCleanToken>(nextAgentDir, true);
        } else {
          if (hasAgentToken<PassiveSegmentToken>(nextAgentDir)) {
            takeAgentToken<PassiveSegmentToken>(nextAgentDir);
            paintFrontSegment(0x696969);
          }
          passAgentToken<PassiveSegmentCleanToken>(nextAgentDir);
          if (hasAgentToken<FinalSegmentCleanToken>(prevAgentDir)) {
            takeAgentToken<FinalSegmentCleanToken>(prevAgentDir);
          }
          passAgentToken<ActiveSegmentCleanToken>(prevAgentDir);
          absorbedActiveToken = true;
          isCoveredCandidate = true;
          agentState = State::Demoted;
          setStateColor();
          return;
        }
      } else {
        passAgentToken<ActiveSegmentToken>(prevAgentDir);
      }
    }
    if (hasAgentToken<CandidacyAnnounceToken>(prevAgentDir)) {
      takeAgentToken<CandidacyAnnounceToken>(prevAgentDir);
      passAgentToken<CandidacyAckToken>(prevAgentDir);
      paintBackSegment(0x696969);
      if (waitingForTransferAck) {
        gotAnnounceBeforeAck = true;
      } else {
        gotAnnounceInCompare = true;
      }
    }
    if (hasAgentToken<SolitudeActiveToken>(prevAgentDir)) {
      passAgentToken<SolitudeActiveToken>(prevAgentDir, takeAgentToken<SolitudeActiveToken>(prevAgentDir));
    }

    if (subPhase == SubPhase::SegmentComparison) {
      if (hasAgentToken<PassiveSegmentToken>(nextAgentDir)) {
        bool isFinalCheck =
            takeAgentToken<PassiveSegmentToken>(nextAgentDir)->isFinal;
        passAgentToken<ActiveSegmentToken>(prevAgentDir, isFinalCheck);
        if (isFinalCheck) {
          paintFrontSegment(0x696969);
        }
      } else if (hasAgentToken<FinalSegmentCleanToken>(prevAgentDir)) {
        bool coveredCandidateCheck =
            takeAgentToken<FinalSegmentCleanToken>(prevAgentDir)->
            hasCoveredCandidate;
        if (!coveredCandidateCheck && !gotAnnounceInCompare) {
          agentState = State::Demoted;
          setStateColor();
        } else {
          subPhase = SubPhase::CoinFlipping;
          setStateColor();
        }
        comparingSegment = false;
        gotAnnounceInCompare = false;
        return;
      } else if (!comparingSegment) {
        passAgentToken<SegmentLeadToken>(nextAgentDir);
        paintFrontSegment(0xff0000);
        comparingSegment = true;
      }
    } else if (subPhase == SubPhase::CoinFlipping) {
      if (hasAgentToken<CandidacyAckToken>(nextAgentDir)) {
        takeAgentToken<CandidacyAckToken>(nextAgentDir);
        paintFrontSegment(0x696969);
        subPhase = SubPhase::SolitudeVerification;
        if (!gotAnnounceBeforeAck) {
          agentState = State::Demoted;
        }
        setStateColor();
        waitingForTransferAck = false;
        gotAnnounceBeforeAck = false;
        return;
      } else if (!waitingForTransferAck && randBool()) {
        passAgentToken<CandidacyAnnounceToken>(nextAgentDir);
        paintFrontSegment(0xffa500);
        waitingForTransferAck = true;
      }
    } else if (subPhase == SubPhase::SolitudeVerification) {
      if (!createdLead) {
        passAgentToken<SolitudeActiveToken>(nextAgentDir,
                                            encodeVector(std::make_pair(1,0)),
                                            localId);
        paintFrontSegment(0x00bfff);
        createdLead = true;
      } else if (hasAgentToken<SolitudeActiveToken>(nextAgentDir)) {
        bool isSole =
            peekAgentToken<SolitudeActiveToken>(nextAgentDir)->isSoleCandidate;
        int id = takeAgentToken<SolitudeActiveToken>(nextAgentDir)->local_id;
        if (isSole && localId == id) {
          agentState = State::SoleCandidate;
        } else {
          subPhase = SubPhase::SegmentComparison;
        }
        createdLead = false;
        return;
      }
    }
  } else if (agentState == State::Demoted) {
    if (hasAgentToken<PassiveSegmentCleanToken>(prevAgentDir)) {
      passiveClean(prevAgentDir);
      passAgentToken<PassiveSegmentCleanToken>(nextAgentDir);
      paintBackSegment(0x696969);
      paintFrontSegment(0x696969);
    }
    if (hasAgentToken<ActiveSegmentCleanToken>(nextAgentDir)) {
      activeClean(nextAgentDir);
      passAgentToken<ActiveSegmentCleanToken>(prevAgentDir);
    }
    if (hasAgentToken<SegmentLeadToken>(prevAgentDir)) {
      takeAgentToken<SegmentLeadToken>(prevAgentDir);
      passAgentToken<SegmentLeadToken>(nextAgentDir);
      passAgentToken<PassiveSegmentToken>(prevAgentDir);
      paintBackSegment(0xff0000);
      paintFrontSegment(0xff0000);
    }
    if (hasAgentToken<PassiveSegmentToken>(nextAgentDir)) {
      bool isFinal = takeAgentToken<PassiveSegmentToken>(nextAgentDir)->isFinal;
      passAgentToken<PassiveSegmentToken>(prevAgentDir, isFinal);
      paintFrontSegment(0x696969);
      paintBackSegment(0x696969);
    }
    if (hasAgentToken<ActiveSegmentToken>(nextAgentDir)) {
      if (!absorbedActiveToken) {
        if (takeAgentToken<ActiveSegmentToken>(nextAgentDir)->isFinal) {
          passAgentToken<FinalSegmentCleanToken>(nextAgentDir);
        } else {
          absorbedActiveToken = true;
        }
      } else {
        bool isFinal =
            takeAgentToken<ActiveSegmentToken>(nextAgentDir)->isFinal;
        passAgentToken<ActiveSegmentToken>(prevAgentDir, isFinal);
      }
    }
    if (hasAgentToken<FinalSegmentCleanToken>(prevAgentDir)) {
      peekAgentToken<FinalSegmentCleanToken>(prevAgentDir)->hasCoveredCandidate
          = isCoveredCandidate;
      absorbedActiveToken = false;
      isCoveredCandidate = false;
      takeAgentToken<FinalSegmentCleanToken>(prevAgentDir);
      passAgentToken<FinalSegmentCleanToken>(nextAgentDir);
    }

    if (hasAgentToken<CandidacyAnnounceToken>(prevAgentDir)) {
      takeAgentToken<CandidacyAnnounceToken>(prevAgentDir);
      passAgentToken<CandidacyAnnounceToken>(nextAgentDir);
      paintBackSegment(0xffa500);
      paintFrontSegment(0xffa500);
    }
    if (hasAgentToken<CandidacyAckToken>(nextAgentDir)) {
      takeAgentToken<CandidacyAckToken>(nextAgentDir);
      passAgentToken<CandidacyAckToken>(prevAgentDir);
      paintFrontSegment(0x696969);
      paintBackSegment(0x696969);
    }
    if (hasAgentToken<SolitudeActiveToken>(prevAgentDir)) {
      if (!hasAgentToken<SolitudeVectorToken>)
    }
  } else if (agentState == State::SoleCandidate) {

  }
}

void LeaderElectionParticle::LeaderElectionAgent::activeClean(int agentDir) {
  if (hasAgentToken<ActiveSegmentToken>(agentDir)) {
    takeAgentToken<ActiveSegmentToken>(agentDir);
  }
  takeAgentToken<ActiveSegmentCleanToken>(agentDir);
  absorbedActiveToken = false;
}

void LeaderElectionParticle::LeaderElectionAgent::passiveClean(int agentDir) {
  if (hasAgentToken<PassiveSegmentToken>(agentDir)) {
    takeAgentToken<PassiveSegmentToken>(agentDir);
  }
  takeAgentToken<PassiveSegmentCleanToken>(agentDir);
}

int LeaderElectionParticle::LeaderElectionAgent::encodeVector(
    std::pair<int, int> vector) const {
  int x = vector.first == -1 ? 2 : vector.first;
  int y = vector.second == -1 ? 2 : vector.second;
  return 10 * x + y;
}

std::pair<int, int> LeaderElectionParticle::LeaderElectionAgent::decodeVector(
    int code) {
  int x = code / 10 == 2 ? -1 : code / 10;
  int y = code % 10 == 2 ? -1 : code % 10;
  return std::make_pair(x, y);
}

std::pair<int, int> LeaderElectionParticle::LeaderElectionAgent::
augmentDirVector(std::pair<int, int> vector, const int offset) {
  const std::array<std::pair<int, int>, 6> vectors =
  { std::make_pair(1, 0), std::make_pair(0, 1), std::make_pair(-1, 1),
    std::make_pair(-1, 0), std::make_pair(0, -1), std::make_pair(1, -1) };

  for (unsigned i = 0; i < vectors.size(); i++) {
    if (vector == vectors.at(i)) {
      return vectors.at((i + offset) % 6);
    }
  }

  Q_ASSERT(false);
  return std::make_pair(0, 0);
}

template <class TokenType>
bool LeaderElectionParticle::LeaderElectionAgent::hasAgentToken(int agentDir) {
  auto prop = [agentDir](const std::shared_ptr<TokenType> token) {
    return token->origin == agentDir;
  };
  return candidateParticle->hasToken<TokenType>(prop);
}

template <class TokenType>
std::shared_ptr<TokenType>
LeaderElectionParticle::LeaderElectionAgent::peekAgentToken(int agentDir) {
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
void LeaderElectionParticle::LeaderElectionAgent::passAgentToken(int agentDir) {
  LeaderElectionParticle* nbr = &candidateParticle->nbrAtLabel(agentDir);
  int origin = -1;
  for (int i = 0; i < 6; i++) {
    if (nbr->hasNbrAtLabel(i) && &nbr->nbrAtLabel(i) == candidateParticle) {
      origin = i;
      break;
    }
  }
  Q_ASSERT(origin != -1);
  nbr->putToken(std::make_shared<TokenType>(origin));
}

template <class TokenType>
void LeaderElectionParticle::LeaderElectionAgent::passAgentToken(int agentDir,
                                                                 bool opt) {
  LeaderElectionParticle* nbr = &candidateParticle->nbrAtLabel(agentDir);
  int origin = -1;
  for (int i = 0; i < 6; i++) {
    if (nbr->hasNbrAtLabel(i) && &nbr->nbrAtLabel(i) == candidateParticle) {
      origin = i;
      break;
    }
  }
  Q_ASSERT(origin != -1);
  nbr->putToken(std::make_shared<TokenType>(origin, opt));
}

template <class TokenType>
void LeaderElectionParticle::LeaderElectionAgent::passAgentToken(int agentDir,
                                                                 int vect,
                                                                 int id) {
  LeaderElectionParticle* nbr = &candidateParticle->nbrAtLabel(agentDir);
  int origin = -1;
  for (int i = 0; i < 6; i++) {
    if (nbr->hasNbrAtLabel(i) && &nbr->nbrAtLabel(i) == candidateParticle) {
      origin = i;
      break;
    }
  }
  Q_ASSERT(origin != -1);
  nbr->putToken(std::make_shared<TokenType>(origin, vect, id));
}

template <class TokenType>
void LeaderElectionParticle::LeaderElectionAgent::passAgentToken(int agentDir,
                                                                 std::shared_ptr<TokenType> token) {
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

void LeaderElectionParticle::LeaderElectionAgent::setStateColor() {
  int globalizedDir = candidateParticle->localToGlobalDir(agentDir);
  if (agentState == State::Candidate) {
    setSubPhaseColor();
  } else if (agentState == State::Demoted) {
    candidateParticle->borderPointColorLabels.at(globalizedDir) = 0x696969;
  } else if (agentState == State::SoleCandidate) {

  } else if (agentState == State::Leader) {

  }
}

void LeaderElectionParticle::LeaderElectionAgent::setSubPhaseColor() {
  int globalizedDir = candidateParticle->localToGlobalDir(agentDir);
  if (subPhase == SubPhase::SegmentComparison) {
    candidateParticle->borderPointColorLabels.at(globalizedDir) = 0xff0000;
  } else if (subPhase == SubPhase::CoinFlipping) {
    candidateParticle->borderPointColorLabels.at(globalizedDir) = 0xffa500;
  } else if (subPhase == SubPhase::SolitudeVerification) {
    candidateParticle->borderPointColorLabels.at(globalizedDir) = 0x00bfff;
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

LeaderElectionSystem::LeaderElectionSystem(int numParticles, double holeProb) {
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

bool LeaderElectionSystem::hasTerminated() const {
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
