/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/demo/tokendemo.h"

TokenDemoParticle::TokenDemoParticle(const Node& head, const int globalTailDir,
                                     const int orientation,
                                     AmoebotSystem& system)
  : AmoebotParticle(head, globalTailDir, orientation, system) {}

void TokenDemoParticle::activate() {
  if (hasToken<DemoToken>()) {
    std::shared_ptr<DemoToken> token = takeToken<DemoToken>();

    // Calculate the direction to pass this token.
    int passTo;
    if (token->_passedFrom == -1) {
      // This hasn't been passed yet; pass red and blue in opposite directions.
      int sweepLen = (std::dynamic_pointer_cast<RedToken>(token)) ? 1 : 2;
      for (int dir = 0; dir < 6; dir++) {
        if (hasNbrAtLabel(dir)) {
          sweepLen--;
          if (sweepLen == 0) {
            passTo = dir;
            break;
          }
        }
      }
    } else {
      // This has been passed before; pass continuing in the same direction.
      for (int offset = 1; offset < 6; offset++) {
        if (hasNbrAtLabel((token->_passedFrom + offset) % 6)) {
          passTo = (token->_passedFrom + offset) % 6;
          break;
        }
      }
    }

    // Update the token's _passedFrom direction. Needs to point at this particle
    // from the perspective of the next neighbor.
    for (int nbrLabel = 0; nbrLabel < 6; nbrLabel++) {
      if (pointsAtMe(nbrAtLabel(passTo), nbrLabel)) {
        token->_passedFrom = nbrLabel;
        break;
      }
    }

    // If the token still has lifetime remaining, pass it on.
    if (token->_lifetime > 0) {
      token->_lifetime--;
      nbrAtLabel(passTo).putToken(token);
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
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
                      + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Local Info:\n";
  text += "  # RedTokens: " + QString::number(countTokens<RedToken>()) + "\n";
  text += "  # BlueTokens: " + QString::number(countTokens<BlueToken>());

  return text;
}

TokenDemoParticle& TokenDemoParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<TokenDemoParticle>(label);
}

TokenDemoSystem::TokenDemoSystem(int numParticles, int lifetime) {
  Q_ASSERT(numParticles >= 6);

  // Instantiate a hexagon of particles.
  int sideLen = static_cast<int>(std::round(numParticles / 6.0));
  Node hexNode = Node(0, 0);
  for (int dir = 0; dir < 6; ++dir) {
    for (int i = 0; i < sideLen; ++i) {
      // Give the first particle five tokens of each color.
      if (hexNode.x == 0 && hexNode.y == 0) {
        auto firstP = new TokenDemoParticle(Node(0, 0), -1, randDir(), *this);
        for (int j = 0; j < 5; ++j) {
          auto redToken = std::make_shared<TokenDemoParticle::RedToken>();
          redToken->_lifetime = lifetime;
          firstP->putToken(redToken);
          auto blueToken = std::make_shared<TokenDemoParticle::BlueToken>();
          blueToken->_lifetime = lifetime;
          firstP->putToken(blueToken);
        }
        insert(firstP);
      } else {
        insert(new TokenDemoParticle(hexNode, -1, randDir(), *this));
      }

      hexNode = hexNode.nodeInDir(dir);
    }
  }
}

bool TokenDemoSystem::hasTerminated() const {
  for (auto p : particles) {
    auto tdp = dynamic_cast<TokenDemoParticle*>(p);
    if (tdp->hasToken<TokenDemoParticle::DemoToken>()) {
      return false;
    }
  }

  return true;
}
