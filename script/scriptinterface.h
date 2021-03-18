/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the API for the scripting interface in the simulator.

#ifndef AMOEBOTSIM_SCRIPT_SCRIPTINTERFACE_H_
#define AMOEBOTSIM_SCRIPT_SCRIPTINTERFACE_H_

#include <QObject>
#include <QString>

#include "core/simulator.h"
#include "script/scriptengine.h"
#include "ui/visitem.h"

class ScriptInterface : public QObject {
  Q_OBJECT

 public:
  explicit ScriptInterface(ScriptEngine& engine, Simulator& sim, VisItem* vis);

 public slots:
  // Script commands. log writes a message to the simulator engine, optionally
  // flagging an error. runScript loads a JavaScript script from the provided
  // filepath and executes it. writeToFile appends the specified text to a file
  // at the given location.
  void log(const QString msg, bool error = false);
  void runScript(const QString scriptFilePath);
  void writeToFile(const QString filePath, const QString text);

  // Simulator flow commands. step executes a single particle activation.
  // setStepDuration sets the simulator's delay between particle activations to
  // the given value; if this value is negative, an error is logged and the step
  // duration is set to 0. runUntilTermination runs the current algorithm
  // instance until its hasTerminated function returns true.
  void step();
  void setStepDuration(const int ms);
  void runUntilTermination();

  // Simulator metrics commands. getNumParticles and getNumObjects return the
  // number of particles and objects in the given instance, respectively.
  // exportMetrics writes the metrics to JSON. See simulator.h for further
  // discussion. getMetric returns either the current value (history = false)
  // or the historical data (history = true) of the metric with parameter-
  // defined name.
  int getNumParticles();
  int getNumObjects();
  void exportMetrics();
  QVariant getMetric(QString name, bool history = false);

  // Visualization commands. focusOn centers the window at the given (x,y) node.
  // setZoom sets the zoom level of the window. saveScreenshot saves the current
  // window as a .png in the specified location; if no filepath is provided, a
  // default path is created that ensures no previous screenshots are
  // overwritten. filmSimulation saves a series of screenshots to the specified
  // location, up to the specified number of steps.
  void setWindowSize(int width = 800, int height = 600);
  void focusOn(int x, int y);
  void setZoom(float zoom);
  void saveScreenshot(QString filePath = "");
  void filmSimulation(QString filePath, const int stepLimit);

 private:
  ScriptEngine& engine;
  Simulator& sim;
  VisItem* vis;

  // Pads the given number with leading zeroes to achieve the specified length.
  QString pad(const int number, const int length);
};

#endif  // AMOEBOTSIM_SCRIPT_SCRIPTINTERFACE_H_
