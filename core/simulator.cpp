/* Copyright (C) 2019 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "core/simulator.h"

#include <QMutexLocker>

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

void Simulator::getData() {
  QMutexLocker locker(&system->mutex);
  system->exportData();
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

QList<QVariant> Simulator::metrics() const {
  QMutexLocker locker(&system->mutex);
  std::vector<std::pair<std::string, double>> metricInfo = system->metrics();
  QList<QVariant> data;
  for(auto const& m: metricInfo) {
    data.push_back(QVariant({QString::fromStdString(m.first), m.second}));
  }
  return data;
}

int Simulator::numObjects() const {
  QMutexLocker locker(&system->mutex);
  return system->numObjects();
}
