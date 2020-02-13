/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "core/amoebotparticle.h"

AmoebotParticle::AmoebotParticle(const Node& head, int globalTailDir,
                                 const int orientation, AmoebotSystem& system)
  : LocalParticle(head, globalTailDir, orientation),
    system(system) {}

AmoebotParticle::~AmoebotParticle() {}

int AmoebotParticle::headMarkGlobalDir() const {
  const int dir = headMarkDir();
  Q_ASSERT(-1 <= dir && dir < 6);

  return (dir == -1) ? -1 : localToGlobalDir(dir);
}

int AmoebotParticle::tailMarkGlobalDir() const {
  const int dir = tailMarkDir();
  Q_ASSERT(-1 <= dir && dir < 6);

  return (dir == -1) ? -1 : localToGlobalDir(dir);
}

int AmoebotParticle::headMarkDir() const {
  return -1;
}

int AmoebotParticle::tailMarkDir() const {
  return -1;
}

bool AmoebotParticle::canExpand(int label) const {
  Q_ASSERT(0 <= label && label < 6);

  return (isContracted() && !hasNbrAtLabel(label) && !hasObjectAtLabel(label));
}

void AmoebotParticle::expand(int label) {
  Q_ASSERT(canExpand(label));

  const int globalExpansionDir = localToGlobalDir(label);
  head = head.nodeInDir(globalExpansionDir);
  globalTailDir = (globalExpansionDir + 3) % 6;
  system.particleMap[head] = this;

  system.registerMovement();
}

bool AmoebotParticle::canPush(int label) const {
  Q_ASSERT(0 <= label && label < 6);

  return (isContracted() && hasNbrAtLabel(label) &&
          nbrAtLabel<Particle>(label).isExpanded());
}

void AmoebotParticle::push(int label) {
  Q_ASSERT(canPush(label));

  const int globalExpansionDir = localToGlobalDir(label);
  const Node handoverNode = head.nodeInDir(globalExpansionDir);
  auto& neighbor = nbrAtLabel<AmoebotParticle>(label);

  head = handoverNode;
  globalTailDir = (globalExpansionDir + 3) % 6;
  system.particleMap[handoverNode] = this;

  if (handoverNode == neighbor.head) {
    neighbor.head = neighbor.tail();
  }
  neighbor.globalTailDir = -1;

  system.registerMovement(2);
  system.registerActivation(&neighbor);
}

void AmoebotParticle::contract(int label) {
  Q_ASSERT(0 <= label && label < 10);
  Q_ASSERT(label == headContractionLabel() || label == tailContractionLabel());

  (label == headContractionLabel()) ? contractHead() : contractTail();
}

void AmoebotParticle::contractHead() {
  Q_ASSERT(isExpanded());

  system.particleMap.erase(head);
  head = tail();
  globalTailDir = -1;

  system.registerMovement();
}

void AmoebotParticle::contractTail() {
  Q_ASSERT(isExpanded());

  system.particleMap.erase(tail());
  globalTailDir = -1;

  system.registerMovement();
}

bool AmoebotParticle::canPull(int label) const {
  Q_ASSERT(0 <= label && label < 10);

  return (isExpanded() && hasNbrAtLabel(label) &&
          nbrAtLabel<Particle>(label).isContracted());
}

void AmoebotParticle::pull(int label) {
  Q_ASSERT(canPull(label));

  const int globalPullDir = labelToGlobalDir(label);
  const Node handoverNode = isHeadLabel(label) ? head : tail();
  auto& neighbor = nbrAtLabel<AmoebotParticle>(label);

  if (isHeadLabel(label)) {
    head = tail();
  }

  globalTailDir = -1;
  neighbor.head = handoverNode;
  neighbor.globalTailDir = globalPullDir;
  system.particleMap[handoverNode] = &neighbor;

  system.registerMovement(2);
  system.registerActivation(&neighbor);
}

bool AmoebotParticle::hasNbrAtLabel(int label) const {
  const Node neighboringNode = nbrNodeReachedViaLabel(label);
  return system.particleMap.find(neighboringNode) != system.particleMap.end();
}

bool AmoebotParticle::hasHeadAtLabel(int label) {
  return hasNbrAtLabel(label) &&
         (nbrAtLabel<Particle>(label).head == nbrNodeReachedViaLabel(label));
}

bool AmoebotParticle::hasTailAtLabel(int label) {
  if (!hasNbrAtLabel(label)) {
    return false;
  }

  const auto& neighbor = nbrAtLabel<Particle>(label);
  if (neighbor.isContracted()) {
    return false;
  }

  return neighbor.tail() == nbrNodeReachedViaLabel(label);
}

bool AmoebotParticle::hasObjectAtLabel(int label) const {
  const Node neighboringNode = nbrNodeReachedViaLabel(label);
  return system.objectMap.find(neighboringNode) != system.objectMap.end();
}

bool AmoebotParticle::hasObjectNbr() const {
  return labelOfFirstObjectNbr() != -1;
}

int AmoebotParticle::labelOfFirstObjectNbr(int startLabel) const {
  const int labelLimit = isContracted() ? 6 : 10;
  for (int labelOffset = 0; labelOffset < labelLimit; labelOffset++) {
    const int label = (startLabel + labelOffset) % labelLimit;
    if (hasObjectAtLabel(label)) {
      return label;
    }
  }

  return -1;
}

void AmoebotParticle::putToken(std::shared_ptr<Token> token) {
  tokens.push_back(token);
}
