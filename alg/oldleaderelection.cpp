#include <QtGlobal>

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
    state(state) {
  borderColorLabels.fill(-1);
  borderPointColorLabels.fill(-1);
}

void LeaderElectionParticle::activate() {
  if (state == State::Finished && !hasToken<LeaderElectionToken>()) {
    return;
  } else if (state == State::Finished && hasToken<LeaderElectionToken>()) {
    // pass token onto the next particle (based on clockwise direction of cycle)
  } else if (state == State::Idle) {
    bool isSurrounded = true;
    for (int dir = 0; dir < 6; dir++) {
      int agentId = 0;
      if (!hasNbrAtLabel(dir) && hasNbrAtLabel((dir + 1) % 6)) {
        Q_ASSERT(agentId < 3);
        isSurrounded = false;

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
    if (isSurrounded) {
      state = State::Finished;
    } else {
      state = State::Candidate;
    }
  } else if (state == State::Candidate) {
    // Check whether or not all of the particle's agents have been demoted
    // If this is the case, then the particle is considered "Finished"
    bool isFinished = true;

    for (unsigned i = 0; i < agents.size(); i++) {
      if (agents.at(i)->agentState != State::Demoted) {
        isFinished = false;
        break;
      }
    }

    if (isFinished) {
      state = State::Finished;
      return;
    }

//    agents.at(currentAgent)->activate();
    currentAgent = (currentAgent + 1) % agents.size();
  }
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
  return;
}

void LeaderElectionParticle::LeaderElectionAgent::setStateColor() {
  if (agentState == State::Candidate) {
    setSubPhaseColor();
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
