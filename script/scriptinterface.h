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
  // LEGACY: getLeaderElectionRounds, getWeakBound, and getStrongBound are only
  // used in LegacySystem and should not be used going forward. TODO: remove
  // after the matching functions in Simulator and System have been removed.
  int getNumParticles();
  int getNumMovements();
  int getNumRounds();
  int getLeaderElectionRounds();
  int getWeakBound();
  int getStrongBound();

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

  // Non-legacy demonstration algorithm instance commands. Documentation for
  // foo() can be found in alg/demo/foo.h.
  void discodemo(const int numParticles = 30, const int counterMax = 5);
  void pulldemo();
  void tokendemo(const int numParticles = 200, const double holeProb = 0.2);

  // Non-legacy algorithm instance commands. Documentation for foo() can be
  // found in alg/foo.h.
  void adder(const int numParticles = 10, int countValue = 250);
  void aggregation(const int numParticles = 2);
  void compaction(const int numParticles = 100, const double holdProb = 0.4);
  void compression(const int numParticles = 100, const double lambda = 4.0);
  void convexhull(const int numParticles = 20, const int numObjects = 200,
                  const double holeProb = 0.1);
  void edgedetect(const int numParticles = 10, int countValue = 250);
  void faultrepair(const int numParticles = 100, const double holeProb = 0.2,
                   const double branchFactor = 0.8);
  void holeelimination(const int numParticles = 100,
                       const double holeProb = 0.4);
  void infobjcoating(const int numParticles = 100, const double holeProb = 0.2);
  void line(const int numParticles = 100, const double holeProb = 0.0);
  void matrix(const int numParticles = 10, int countValue = 250,
              int whichStream = 3, const int mode = 0);
  void rectangle(const int numParticles = 200, const double holeProb = 0.2);
  void ring(const int numParticles = 100, const double holeProb = 0.0);
  void shapeformation(const int numParticles = 200, const double holeProb = 0.2,
                      const QString mode = "h");
  void sierpinski(const int numParticles = 200, const double holeProb = 0.2);
  void swarmseparation(const int numParticles = 200, const double c_rand = 0.7,
                       const double c_repulse = 0.5);

  // Legacy algorithm instance commands. Documentation for foo() can be found in
  // alg/legacy/foo.h.
  void boundedobjcoating(const int numStaticParticles, const int numParticles,
                         const double holeProb = 0.2);
  void leaderelection(const uint numParticles = 100);
  void leaderelectiondemo();
  void universalcoating(const int staticParticlesRadius = 5,
                        const int numParticles = 50,
                        const double holeProb = 0.2);

  // Commands for universal coating competitive analysis. TODO: when bringing
  // Universal Coating out of legacy, figure out what to do with these.
  // int getUniversalCoatingWeakLowerBound();
  // int getUniversalCoatingStrongLowerBound();

 private:
  ScriptEngine& engine;
  Simulator& sim;
  VisItem* vis;

  // Pads the given number with leading zeroes to achieve the specified length.
  QString pad(const int number, const int length);
};

#endif  // AMOEBOTSIM_SCRIPT_SCRIPTINTERFACE_H
