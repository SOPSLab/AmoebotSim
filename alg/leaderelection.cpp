#include <QtGlobal>

#include <set>
#include <vector>

#include "alg/leaderelection.h"

LeaderElectionParticle::LeaderElectionParticle(const Node head,
                                               const int globalTailDir,
                                               const int orientation,
                                               AmoebotSystem& system,
                                               State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state) {

}

void LeaderElectionParticle::activate() {

}

int LeaderElectionParticle::headMarkColor() const {
  switch(state) {
    case State::Candidate:   return 0x00ff00;
    case State::SoleCandidate: return 0x0000ff;
    case State::Idle:   return -1;
    case State::Leader: return 0xffff00;
    case State::Finished: return 0xff0000;
    default: return -1;
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

LeaderElectionSystem::LeaderElectionSystem(int numParticles, double holeProb,
                                           QString mode) {
  Q_ASSERT(mode == "h" || mode == "s" || mode == "t1" || mode == "t2");
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
    if (hp->state != LeaderElectionParticle::State::Seed &&
        hp->state != LeaderElectionParticle::State::Finish) {
      return false;
    }
  }

  return true;
}

std::set<QString> LeaderElectionSystem::getAcceptedModes() {
  std::set<QString> set = {"h", "t1", "t2", "s"};
  return set;
}
