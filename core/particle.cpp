/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "core/particle.h"

#include <QtGlobal>

Particle::Particle(const Node& head, int globalTailDir)
  : head(head),
    globalTailDir(globalTailDir) {
  Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
}

bool Particle::isContracted() const {
  return (globalTailDir == -1);
}

bool Particle::isExpanded() const {
  return !isContracted();
}

Node Particle::tail() const {
  Q_ASSERT(isExpanded());
  Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);

  return head.nodeInDir(globalTailDir);
}

int Particle::headMarkColor() const {
  return -1;
}

int Particle::headMarkGlobalDir() const {
  return -1;
}

int Particle::tailMarkColor() const {
  return -1;
}

int Particle::tailMarkGlobalDir() const {
  return -1;
}

std::array<int, 18> Particle::borderColors() const {
  std::array<int, 18> borderColors;
  borderColors.fill(-1);

  return borderColors;
}

std::array<int, 6> Particle::borderPointColors() const {
  std::array<int, 6> borderPointColors;
  borderPointColors.fill(-1);

  return borderPointColors;
}

QString Particle::inspectionText() const {
  return "Overwrite Particle::inspectionText() to specify an inspection text.";
}
