/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#ifndef AMOEBOTSIM_CORE_SIMULATOR_H_
#define AMOEBOTSIM_CORE_SIMULATOR_H_

#include <memory>

#include <QObject>
#include <QTimer>
#include <QVariant>

#include "core/system.h"

class Simulator : public QObject {
  Q_OBJECT

 public:
  Simulator();
  virtual ~Simulator();

  void setSystem(std::shared_ptr<System> _system);
  std::shared_ptr<System> getSystem() const;

 signals:
  void systemChanged(std::shared_ptr<System> _system);
  void stepDurationChanged(int ms);
  void saveScreenshot(const QString filePath);

  void started();
  void stopped();

 public slots:
  // Responds to control flow signals from the GUI and scripts. Start, stop, and
  // step are self-explanatory. stepForParticleAt executes one activation for
  // the specific particle at the given node. setStepDuration updates the delay
  // in milliseconds between particle activations. runUntilTermination activates
  // particles repeatedly until the hasTerminated condition is satisfied.
  void start();
  void stop();
  void step();
  void stepForParticleAt(Node node);
  void setStepDuration(int ms);
  void runUntilTermination();

  // Responds to GUI and script requests for statistics and metrics.
  int numParticles() const;
  int numObjects() const;
  QVariant metrics() const;

  // Responds to the exportMetrics signal from the GUI and scripts by creating
  // an output file with a unique timestamp (to avoid accidental overwrites) and
  // writing the metrics JSON to it.
  void exportMetrics();

  // Emits a signal that updates the system visually, followed by a signal that
  // takes a screenshot of the result.
  void saveScreenshotSetup(const QString filePath);

 protected:
  QTimer stepTimer;
  std::shared_ptr<System> system;
};

#endif  // AMOEBOTSIM_CORE_SIMULATOR_H_
