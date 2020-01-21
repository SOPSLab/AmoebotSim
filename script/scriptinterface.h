/* Copyright (C) 2019 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the API for the command line interface in the simulator.

#ifndef AMOEBOTSIM_SCRIPT_SCRIPTINTERFACE_H
#define AMOEBOTSIM_SCRIPT_SCRIPTINTERFACE_H

#include <QObject>
#include <QString>

#include "script/scriptengine.h"
#include "core/simulator.h"
#include "ui/visitem.h"

class ScriptInterface : public QObject {
  Q_OBJECT

 public:
  explicit ScriptInterface(ScriptEngine& engine, Simulator& sim, VisItem* vis);

 public slots:
  // These public slots functions are directly accessible from the simulator's
  // command line.

  // Script commands. log writes a message to the simulator engine, optionally
  // flagging an error. runScript loads a JavaScript script from the provided
  // filepath and executes it in the simulator command line. writeToFile appends
  // the specified text to a file at the given location.
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

  // Simulator metrics commands. getNumParticles returns the number of particles
  // in the given instance. getNumMovements returns the total number of
  // expansions and contractions executed by the particle system. getNumRounds
  // returns the number of asynchronous rounds completed by the particle system.
  // See amoebotsystem.h for further discussion.
  int getNumParticles();
  int getNumMovements();
  int getNumRounds();

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

  // Demonstration algorithm instance commands. Documentation for foo() can be
  // found in alg/demo/foo.h.
  void discodemo(const int numParticles = 30, const int counterMax = 5);
  void pulldemo();
  void tokendemo(const int numParticles = 200, const double holeProb = 0.2);

  // Algorithm instance commands. Documentation for foo() can be found in
  // alg/foo.h.
  void compression(const int numParticles = 100, const double lambda = 4.0);
  void improvedleaderelection(const int numParticles = 100,
                              const double holeProb = 0.2);
  void infobjcoating(const int numParticles = 100, const double holeProb = 0.2);
  void leaderelection(const int numParticles = 100,
                      const double holeProb = 0.2);
  void shapeformation(const int numParticles = 200, const double holeProb = 0.2,
                      const QString mode = "h");

 private:
  ScriptEngine& engine;
  Simulator& sim;
  VisItem* vis;

  // Pads the given number with leading zeroes to achieve the specified length.
  QString pad(const int number, const int length);
};

#endif  // AMOEBOTSIM_SCRIPT_SCRIPTINTERFACE_H
