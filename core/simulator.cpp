#include <QMutexLocker>

#include "core/simulator.h"
#include "script/scriptinterface.h"

Simulator::Simulator() {
  stepTimer.setInterval(100);
  connect(&stepTimer, &QTimer::timeout, this, &Simulator::step);
}

Simulator::~Simulator() {
  stepTimer.stop();
}

void Simulator::setSystem(std::shared_ptr<System> _system) {
  stepTimer.stop();
  emit stopped();

  system = _system;
  emit systemChanged(system);
}

std::shared_ptr<System> Simulator::getSystem() const {
  return system;
}

void Simulator::start() {
  stepTimer.start();
  emit started();
}

void Simulator::stop() {
  stepTimer.stop();
  emit stopped();
}

void Simulator::step() {
  QMutexLocker locker(&system->mutex);
  system->activate();

  if (system->hasTerminated()) {
    stop();
  }
}

void Simulator::stepForParticleAt(Node node) {
  QMutexLocker locker(&system->mutex);
  system->activateParticleAt(node);
}

void Simulator::setStepDuration(int ms) {
  stepTimer.setInterval(ms);
  emit stepDurationChanged(ms);
}

void Simulator::runUntilTermination() {
  QMutexLocker locker(&system->mutex);
  while (!system->hasTerminated()) {
    system->activate();
  }
}

void Simulator::saveScreenshotSetup(const QString filePath) {
  emit systemChanged(system);
  emit saveScreenshot(filePath);
}

int Simulator::numParticles() const {
  QMutexLocker locker(&system->mutex);
  return system->size();
}

int Simulator::numObjects() const {
  QMutexLocker locker(&system->mutex);
  return system->numObjects();
}

int Simulator::numMovements() const {
  QMutexLocker locker(&system->mutex);
  return system->numMovements();
}

int Simulator::numRounds() const {
  QMutexLocker locker(&system->mutex);
  return system->numRounds();
}
