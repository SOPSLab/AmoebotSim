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
    putToken(std::make_shared<RedToken>());
    putToken(std::make_shared<RedToken>());
    putToken(std::make_shared<BlueToken>());
    putToken(std::make_shared<BlueToken>());
    putToken(std::make_shared<BlueToken>());
    putToken(std::make_shared<BlueToken>());
    putToken(std::make_shared<BlueToken>());
  }
}

void TokenDemoParticle::activate() {
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

TokenDemoSystem::TokenDemoSystem(int numParticles) {
  Q_ASSERT(numParticles > 0);

  // Insert the seed at (0, 0).
  insert(new TokenDemoParticle(Node(0, 0), -1, randDir(), *this,
                               ShapeFormationParticle::State::Seed, "l"));

  int sideLen = static_cast<int>(std::round(1.4 * std::sqrt(numParticles)));
  Node boundNode = Node(0, 0);
  for (int dir = 0; dir < 6; ++dir) {
    for (int i = 0; i < sideLen; ++i) {

      if(dir == 5 && i == sideLen - 1) {
        return;
      }

      boundNode = boundNode.nodeInDir(dir);
      insert(new TokenDemoParticle(boundNode, -1, randDir(), *this,
                                   ShapeFormationParticle::State::Finish, "l"));
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
