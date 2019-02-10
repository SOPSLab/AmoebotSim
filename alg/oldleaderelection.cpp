#include <QtGlobal>

#include <set>
#include <vector>

#include "alg/oldleaderelection.h"

LeaderElectionParticle::LeaderElectionParticle(const Node head,
                                               const int globalTailDir,
                                               const int orientation,
                                               AmoebotSystem& system,
                                               State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state) {

}

void LeaderElectionParticle::activate() {
  if (state == State::Finished && !hasToken<LeaderElectionToken>()) {
    return;
  } else if (state == State::Finished && hasToken<LeaderElectionToken>()) {
    // pass token onto the next particle (based on clockwise direction of cycle)
  } else if (state == State::Idle) {
    for (int i = 0; i < 6; i++) {
      if (!hasNbrAtLabel(i) && hasNbrAtLabel((i + 1) % 6)) {
        Q_ASSERT(agents.size() <= 3);
        // Insert agent into vector with defined parameters
      }
    }
  } else if (state == State::Candidate) {
    // Check whether or not all of the particle's agents have been demoted
    // If this is the case, then the particle is considered "Finished"
    bool isFinished = true;

    for (unsigned i = 0; i < agents.size(); i++) {
      if (agents.at(i).state != State::Demoted) {
        isFinished = false;
        break;
      }
    }

    if (isFinished) {
      state = State::Finished;
      return;
    }

//    agents.at(currentAgent).activate();
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
      case State::Idle:   return "seed";
      case State::Candidate:   return "candidate";
      case State::SoleCandidate: return "sole candidate";
      case State::Demoted:   return "demoted";
      case State::Leader: return "leader";
      case State::Finished: return "finished";
      default:            return "no state";
    }
  }();
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

void LeaderElectionParticle::LeaderElectionAgent::paintFrontSegment(
    const int color) {
  unsigned tempDir = agentDir;
  while (tempDir != (nextAgentDir + 1) % 6) {
      if ((tempDir + 5) % 6 != nextAgentDir) {
          candidateParticle->borderColorLabels.at((3 * tempDir + 17) % 18) =
              color;
      }
      tempDir = (tempDir + 5) % 6;
  }
}

void LeaderElectionParticle::LeaderElectionAgent::paintBackSegment(
    const int color) {
  candidateParticle->borderColorLabels.at(3 * agentDir + 1) = color;
}

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
