/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "core/simulator.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutexLocker>
#include <QTextStream>
#include <QtGlobal>

#include "core/metric.h"

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

int Simulator::numParticles() const {
  QMutexLocker locker(&system->mutex);
  return system->size();
}

int Simulator::numObjects() const {
  QMutexLocker locker(&system->mutex);
  return system->numObjects();
}

QVariant Simulator::metrics() const {
  QMutexLocker locker(&system->mutex);
  QList<QVariant> metricsData;
  for (const auto& c : system->getCounts()) {
    metricsData.push_back(QVariant({c->_name, c->_value}));
  }
  for (const auto& m : system->getMeasures()) {
    if (m->_history.empty()) {
      metricsData.push_back(QVariant({m->_name, 0.0}));
    } else {
      metricsData.push_back(QVariant({m->_name, m->_history.back()}));
    }
  }
  return QVariant::fromValue(metricsData);
}

void Simulator::exportMetrics() {
  QMutexLocker locker(&system->mutex);
  QDir metricsDir(QCoreApplication::applicationDirPath());
  #ifdef Q_OS_MACOS
    metricsDir.cd("../../..");  // Escape the macOS application bundle.
  #endif
  if (!metricsDir.cd("metrics")) {
    metricsDir.mkdir("metrics");
    metricsDir.cd("metrics");
  }
  QFile outFile(metricsDir.path() + "/metrics_" +
                QString::number(QDateTime::currentSecsSinceEpoch()) + ".json");
  if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return;
  }
  QTextStream outStream(&outFile);
  outStream << system->metricsAsJSON();
  outFile.close();
}

void Simulator::saveScreenshotSetup(const QString filePath) {
  emit systemChanged(system);
  emit saveScreenshot(filePath);
}
