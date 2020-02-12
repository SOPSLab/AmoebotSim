/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include <memory>

#include "alg/demo/tokendemo.h"

TokenDemoParticle::TokenDemoParticle(const Node head, const int globalTailDir,
                                     const int orientation,
                                     AmoebotSystem& system, State state,
                                     const QString mode)
  : ShapeFormationParticle(head, globalTailDir, orientation, system, state,
                           mode) {
  // Initialize the seed particle to hold three red tokens and two blue.
  if (state == State::Seed) {
    putToken(std::make_shared<RedToken>());
    putToken(std::make_shared<RedToken>());
    putToken(std::make_shared<RedToken>());
    putToken(std::make_shared<BlueToken>());
    putToken(std::make_shared<BlueToken>());
  }
}

void TokenDemoParticle::activate() {
  ShapeFormationParticle::activate();

  // If this particle is holding a token and is the seed or is finished, choose
  // a random seed or finished neighbor. If such a neighbor exists, take the
  // first token this particle is holding and give it to the chosen neighbor.
  if (hasToken<Token>()) {
    if (isContracted() && (state == State::Seed || state == State::Finish)) {
      int lbl = labelOfFirstNbrInState({State::Seed, State::Finish}, randDir());
      if (lbl != -1) {
        nbrAtLabel(lbl).putToken(takeToken<Token>());
      }
    }
  }
}

int TokenDemoParticle::headMarkColor() const {
  if (hasToken<RedToken>() && hasToken<BlueToken>()) {
    return 0xff00ff;
  } else if (hasToken<RedToken>()) {
    return 0xff0000;
  } else if (hasToken<BlueToken>()) {
    return 0x0000ff;
  } else if (state == State::Seed || state == State::Finish) {
    return 0x000000;
  } else {
    return -1;
  }
}

QString TokenDemoParticle::inspectionText() const {
  QString text = ShapeFormationParticle::inspectionText();
  text += "numRedTokens: " + QString::number(countTokens<RedToken>());
  text += "\n";
  text += "numBlueTokens: " + QString::number(countTokens<BlueToken>());

  return text;
}

TokenDemoParticle& TokenDemoParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<TokenDemoParticle>(label);
}

TokenDemoSystem::TokenDemoSystem(int numParticles, double holeProb) {
  Q_ASSERT(numParticles > 0);
  Q_ASSERT(0 <= holeProb && holeProb <= 1);

  // Insert the seed at (0, 0).
  insert(new TokenDemoParticle(Node(0, 0), -1, randDir(), *this,
                               ShapeFormationParticle::State::Seed, "l"));

  std::set<Node> occupied;
  occupied.insert(Node(0, 0));

  std::set<Node> candidates;
  for(int i = 0; i < 6; i++) {
    candidates.insert(Node(0, 0).nodeInDir(i));
  }

  // Add inactive particles.
  int numNonStaticParticles = 0;
  while(numNonStaticParticles < numParticles && !candidates.empty()) {
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
      insert(new TokenDemoParticle(randomCandidate, -1, randDir(), *this,
                                   ShapeFormationParticle::State::Idle, "l"));
      numNonStaticParticles++;

      // Add new candidates.
      for (int i = 0; i < 6; i++) {
        auto neighbor = randomCandidate.nodeInDir(i);
        if (occupied.find(neighbor) == occupied.end()) {
          candidates.insert(neighbor);
        }
      }
    }
  }
}

bool TokenDemoSystem::hasTerminated() const {
  #ifdef QT_DEBUG
    if (!isConnected(particles)) {
      return true;
    }
  #endif

  return false;
}
