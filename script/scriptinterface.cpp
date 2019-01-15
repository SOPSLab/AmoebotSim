#include <QFile>
#include <QTextStream>
#include <QTime>
#include <math.h>

#include "alg/demo/discodemo.h"
#include "alg/demo/pulldemo.h"
#include "alg/demo/tokendemo.h"
#include "alg/legacy/boundedobjcoating.h"
#include "alg/legacy/leaderelection.h"
#include "alg/legacy/leaderelectiondemo.h"
#include "alg/legacy/legacysystem.h"
#include "alg/ring.h"
#include "alg/legacy/universalcoating.h"
#include "alg/aggregation.h"
#include "alg/compression.h"
#include "alg/convexhull.h"
#include "alg/edgedetect.h"
#include "alg/infobjcoating.h"
#include "alg/line.h"
#include "alg/matrix.h"
#include "alg/matrix2.h"
#include "alg/shapeformation.h"
#include "alg/swarmseparation.h"

#include "script/scriptinterface.h"
#include "core/node.h"

// TODO: See comment in scriptinterface.h regarding universal coating competitve
// analysis functions.
// #include "helper/universalcoatinghelper.h"

ScriptInterface::ScriptInterface(ScriptEngine &engine, Simulator& sim,
                                 VisItem *vis)
  : engine(engine),
    sim(sim),
    vis(vis) {
  shapeformation();
}

void ScriptInterface::log(const QString msg, bool error) {
  emit engine.log(msg, error);
}

void ScriptInterface::runScript(const QString scriptFilePath) {
  engine.runScript(scriptFilePath);
}

void ScriptInterface::writeToFile(const QString filePath, const QString text) {
  QFile file(filePath);

  if (!file.open(QFile::WriteOnly | QFile::Append)) {
    log("Could not write to file", true);
    return;
  }

  QTextStream stream(&file);
  stream << text;

  file.close();
}

void ScriptInterface::step() {
  sim.step();
}

void ScriptInterface::runUntilTermination() {
  sim.runUntilTermination();
}

void ScriptInterface::setStepDuration(const int ms) {
  if (ms < 0) {
    log("Step duration must be non-negative", true);
    sim.setStepDuration(0);
  } else {
    sim.setStepDuration(ms);
  }
}

int ScriptInterface::getNumParticles() {
  return sim.numParticles();
}

int ScriptInterface::getNumMovements() {
  return sim.numMovements();
}

int ScriptInterface::getNumRounds() {
  return sim.numRounds();
}

int ScriptInterface::getLeaderElectionRounds() {
  return sim.leaderElectionRounds();
}

int ScriptInterface::getWeakBound() {
  return sim.weakBounds();
}

int ScriptInterface::getStrongBound() {
  return sim.strongBounds();
}

void ScriptInterface::setWindowSize(int width, int height) {
  if(vis != nullptr) {
    vis->setWindowSize(width, height);
  }
}


void ScriptInterface::focusOn(int x, int y) {
  if (vis != nullptr) {
    vis->focusOn(Node(x, y));
  }
}

void ScriptInterface::setZoom(float zoom) {
  if(vis != nullptr) {
    vis->setZoom(zoom);
  }
}

void ScriptInterface::saveScreenshot(QString filePath) {
  if(filePath == "") {
    filePath = QString("amoebotsim_") +
               QString::number(QTime::currentTime().msecsSinceStartOfDay()) +
               QString(".png");
  }

  sim.saveScreenshotSetup(filePath);
}

void ScriptInterface::filmSimulation(QString filePath, const int stepLimit) {
  int fnameLen = 0;
  int temp = stepLimit;
  while (temp >= 10) {
    ++fnameLen;
    temp = temp % 10;
  }

  int i = 0;
  while(!sim.getSystem()->hasTerminated() && i < stepLimit) {
    emit vis->beforeRendering();  // Updates GUI #rounds and #movements labels.
    saveScreenshot(filePath + pad(i,fnameLen) + QString(".png"));
    step();
    ++i;
  }
}

void ScriptInterface::discodemo(const int numParticles, const int counterMax) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (counterMax <= 0) {
    log("counterMax must be > 0", true);
  } else {
    sim.setSystem(std::make_shared<DiscoDemoSystem>(numParticles));
  }
}

void ScriptInterface::pulldemo() {
  sim.setSystem(std::make_shared<PullDemoSystem>());
}

void ScriptInterface::tokendemo(const int numParticles, const double holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<TokenDemoSystem>(numParticles, holeProb));
  }
}

void ScriptInterface::aggregation(const int numParticles) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else {
    sim.setSystem(std::make_shared<AggregateSystem>(numParticles));
  }
}

void ScriptInterface::compression(const int numParticles, const double lambda) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else {
    sim.setSystem(std::make_shared<CompressionSystem>(numParticles, lambda));
  }
}

void ScriptInterface::convexhull(const int numParticles, const int numObjects,
                                 const double holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (numObjects <= 0) {
    log("# object objects must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<ConvexHullSystem>(numParticles, numObjects,
                                                     holeProb));
  }
}

void ScriptInterface::edgedetect(const int numParticles, int countValue) {
  // TODO: check for constraints on countValue.
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else {
    sim.setSystem(std::make_shared<EdgeDetectSystem>(numParticles, countValue));
  }
}

void ScriptInterface::infobjcoating(const int numParticles,
                                    const double holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<InfObjCoatingSystem>(numParticles,
                                                        holeProb));
  }
}

void ScriptInterface::line(const int numParticles, const double holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<LineSystem>(numParticles, holeProb));
  }
}

void ScriptInterface::matrix(const int numParticles, int countValue,
                             int whichStream, const int mode) {
  // TODO: check for constraints on countValue and whichStream.
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (mode == 0) {
    sim.setSystem(std::make_shared<Matrix2System>(numParticles, countValue,
                                                  whichStream));
  } else if (mode == 1) {
    sim.setSystem(std::make_shared<MatrixSystem>(numParticles, countValue));
  }
}

void ScriptInterface::ring(const int numParticles, const double holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<RingSystem>(numParticles, holeProb));
  }
}

void ScriptInterface::shapeformation(const int numParticles,
                                     const double holeProb,
                                     const QString mode) {
  std::set<QString> set = ShapeFormationSystem::getAcceptedModes();
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else if (set.find(mode) == set.end()) {
    QString accepted = "";
    for(std::set<QString>::iterator it = set.begin(); it != set.end(); ++it) {
      if (accepted != "") accepted = accepted + ", " + *it;
      else accepted = *it;
    }
    log("only accepted modes are: " + accepted, true);
  } else {
    sim.setSystem(std::make_shared<ShapeFormationSystem>(numParticles, holeProb,
                                                         mode));
  }
}

void ScriptInterface::swarmseparation(const int numParticles,
                                      const double c_rand,
                                      const double c_repulse) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (c_rand < 0) {
    log("c_rand >= 0 required", true);
  } else if (c_repulse < 0) {
    log("c_repulse >= 0 required", true);
  } else {
    sim.setSystem(std::make_shared<SwarmSeparationSystem>(numParticles, c_rand,
                                                          c_repulse));
  }
}

void ScriptInterface::boundedobjcoating(const int numStaticParticles,
                                        const int numParticles,
                                        const double holeProb) {
  // TODO: check for constraints on numStaticParticles.
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(BoundedObjCoating::BoundedObjCoating::instance(
        numStaticParticles, numParticles, holeProb));
  }
}

void ScriptInterface::leaderelection(const uint numParticles) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else {
    sim.setSystem(LeaderElection::LeaderElection::instance(numParticles));
  }
}

void ScriptInterface::leaderelectiondemo() {
  sim.setSystem(LeaderElectionDemo::LeaderElectionDemo::instance());
}

void ScriptInterface::universalcoating(const int staticParticlesRadius,
                                       const int numParticles,
                                       const double holeProb) {
  // TODO: rename 'static particles' to 'object'.
  if (staticParticlesRadius <= 0) {
    log("radius of static particles must be > 0", true);
  } else if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(UniversalCoating::UniversalCoating::instance(
        staticParticlesRadius, numParticles, holeProb));
  }
}

/* int ScriptInterface::getUniversalCoatingWeakLowerBound() {
  return UniversalCoating::getWeakLowerBound(*sim.getSystem());
}

int ScriptInterface::getUniversalCoatingStrongLowerBound() {
  return UniversalCoating::getStrongLowerBound(*sim.getSystem());
} */

QString ScriptInterface::pad(const int number, const int length) {
  QString str = "" + QString::number(number);

  while(str.length() < length) {
    str = QString("0") + str;
  }

  return str;
}
