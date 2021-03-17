/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

# include "alg/demo/dynamicdemo.h"

DynamicDemoParticle::DynamicDemoParticle(const Node& head,
                                         const int globalTailDir,
                                         const int orientation,
                                         AmoebotSystem& system,
                                         const double growProb,
                                         const double dieProb)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      _growProb(growProb),
      _dieProb(dieProb) {}

void DynamicDemoParticle::activate() {
  // With the specified growth probability, choose a random direction and add
  // a particle in the incident node if it is unoccupied.
  if (randDouble(0, 1) < _growProb) {
    int growDir = randDir();
    if (!hasNbrAtLabel(growDir)) {
      system.insert(new DynamicDemoParticle(
                      head.nodeInDir(localToGlobalDir(growDir)), -1, randDir(),
                      system, _growProb, _dieProb));
    }
  }

  // With the specified death probability, die (i.e., remove this particle).
  if (randDouble(0, 1) < _dieProb) {
    system.remove(this);
  }
}

QString DynamicDemoParticle::inspectionText() const {
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
                      + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Local Info:\n";
  text += "  growProb: " + QString::number(_growProb) + "\n";
  text += "  dieProb: " + QString::number(_dieProb);

  return text;
}

DynamicDemoSystem::DynamicDemoSystem(unsigned int numParticles, double growProb,
                                     double dieProb) {
  // Instantiate the system in the shape of a hexagon.
  int x, y;
  for (unsigned int i = 1; i <= numParticles; ++i) {
    int layer = 1;
    int position = i - 1;
    while (position - (6 * layer) >= 0) {
      position -= 6 * layer;
      ++layer;
    }

    switch(position / layer) {
      case 0: {
        x = layer;
        y = (position % layer) - layer;
        if (position % layer == 0) {x -= 1; y += 1;}  // Corner case.
        break;
      }
      case 1: {
        x = layer - (position % layer);
        y = position % layer;
        break;
      }
      case 2: {
        x = -1 * (position % layer);
        y = layer;
        break;
      }
      case 3: {
        x = -1 * layer;
        y = layer - (position % layer);
        break;
      }
      case 4: {
        x = (position % layer) - layer;
        y = -1 * (position % layer);
        break;
      }
      case 5: {
        x = (position % layer);
        y = -1 * layer;
        break;
      }
    }

    insert(new DynamicDemoParticle(Node(x, y), -1, randDir(), *this, growProb,
                                   dieProb));
  }
}

bool DynamicDemoSystem::hasTerminated() const {
  return particles.size() == 0;
}
