#include <QtGlobal>

#include "core/particle.h"

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
