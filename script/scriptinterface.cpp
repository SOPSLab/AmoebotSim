#include <QFile>
#include <QTextStream>
#include <QTime>
#include <math.h>

#include "alg/demo/pulldemo.h"
#include "alg/demo/tokendemo.h"
#include "alg/legacy/boundedobjcoating.h"
#include "alg/legacy/leaderelection.h"
#include "alg/legacy/leaderelectiondemo.h"
#include "alg/legacy/legacysystem.h"
#include "alg/ring.h"
#include "alg/legacy/universalcoating.h"
#include "alg/2sitecbridge.h"
#include "alg/adder.h"
#include "alg/compaction.h"
#include "alg/compression.h"
#include "alg/convexhull.h"
#include "alg/edgedetect.h"
#include "alg/faultrepair.h"
#include "alg/holeelimination.h"
#include "alg/infobjcoating.h"
#include "alg/ising.h"
#include "alg/line.h"
#include "alg/linesort.h"
#include "alg/matrix.h"
#include "alg/matrix2.h"
#include "alg/rectangle.h"
#include "alg/shapeformation.h"
#include "alg/sierpinski.h"
#include "alg/twositeebridge.h"

#include "script/scriptinterface.h"
#include "sim/node.h"

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

void ScriptInterface::round() {
  sim.round();
}

void ScriptInterface::runUntilTermination() {
  sim.runUntilTermination();
}

void ScriptInterface::setRoundDuration(const int ms) {
  if (ms < 0) {
    log("Round duration must be non-negative", true);
    sim.setRoundDuration(0);
  } else {
    sim.setRoundDuration(ms);
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

void ScriptInterface::filmSimulation(QString filePath, const int roundLimit) {
  int fnameLen = 0;
  int temp = roundLimit;
  while (temp >= 10) {
    ++fnameLen;
    temp = temp % 10;
  }

  int i = 0;
  while(!sim.getSystem()->hasTerminated() && i < roundLimit) {
    emit vis->beforeRendering();  // Updates GUI #rounds and #movements labels.
    saveScreenshot(filePath + pad(i,fnameLen) + QString(".png"));
    round();
    ++i;
  }
}

void ScriptInterface::pulldemo() {
  sim.setSystem(std::make_shared<PullDemoSystem>());
}

void ScriptInterface::tokendemo(const int numParticles, const float holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<TokenDemoSystem>(numParticles, holeProb));
  }
}

void ScriptInterface::adder(const int numParticles, int countValue) {
  // TODO: check on any constraints on countValue.
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else {
    sim.setSystem(std::make_shared<AdderSystem>(numParticles, countValue));
  }
}

void ScriptInterface::compaction(const int numParticles, const float holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<CompactionSystem>(numParticles, holeProb));
  }
}

void ScriptInterface::compression(const int numParticles, const float lambda) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else {
    sim.setSystem(std::make_shared<CompressionSystem>(numParticles, lambda));
  }
}

void ScriptInterface::convexhull(const int numParticles, const int numTiles,
                                 const float holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (numTiles <= 0) {
    log("# object tiles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<ConvexHullSystem>(numParticles, numTiles,
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

void ScriptInterface::faultrepair(const int numParticles,
                                  const float holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<FaultRepairSystem>(numParticles, holeProb));
  }
}

void ScriptInterface::holeelimination(const int numParticles,
                                      const float holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<HoleEliminationSystem>(numParticles,
                                                          holeProb));
  }
}

void ScriptInterface::infobjcoating(const int numParticles,
                                    const float holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<InfObjCoatingSystem>(numParticles,
                                                        holeProb));
  }
}

void ScriptInterface::ising(const int numParticles, const float beta) {
  // TODO: check on any constraints for beta.
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else {
    sim.setSystem(std::make_shared<IsingSystem>(numParticles, beta));
  }
}

void ScriptInterface::line(const int numParticles, const float holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<LineSystem>(numParticles, holeProb));
  }
}

void ScriptInterface::linesort(const int numParticles, const float holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<LineSortSystem>(numParticles, holeProb));
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

void ScriptInterface::rectangle(const int numParticles, const float holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<RectangleSystem>(numParticles, holeProb));
  }
}

void ScriptInterface::shapeformation(const int numParticles,
                                     const float holeProb, const QString mode) {
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

void ScriptInterface::sierpinski(const int numParticles, const float holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<SierpinskiSystem>(numParticles, holeProb));
  }
}

void ScriptInterface::twositecbridge(const int numParticles, const float lambda,
                                     const float alpha) {
  if (numParticles < 5) {
    log("# particles must be >= 5", true);
  } else if (lambda <= 1) {
    log("lambda > 1 required", true);
  } else if (alpha < 1) {
    log("alpha >= 1 required", true);
  } else {
    sim.setSystem(std::make_shared<TwoSiteCBridgeSystem>(numParticles, lambda,
                                                         alpha));
  }
}

void ScriptInterface::twositeebridge(const int numParticles,
                                     const float explambda,
                                     const float complambda,
                                     const float siteDistance) {
  if (numParticles < 5) {
    log("# particles must be >= 5", true);
  } else if (explambda <= 0 || explambda >= 2.17) {
    log("expansion lambda must be > 0 and < 2.17", true);
  } else if (complambda <= 3.42) {
    log("compression lambda must be > 3.42", true);
  } else if (siteDistance <= 1 || siteDistance > sqrt(numParticles)) {
    log("site distance factor must be > 1 and < sqrt(#particles)", true);
  } else {
    sim.setSystem(std::make_shared<TwoSiteEBridgeSystem>(numParticles,
                                                         explambda, complambda,
                                                         siteDistance));
  }
}

void ScriptInterface::boundedobjcoating(const int numStaticParticles,
                                        const int numParticles,
                                        const float holeProb) {
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

void ScriptInterface::leaderelection(const int numParticles) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else {
    sim.setSystem(LeaderElection::LeaderElection::instance(numParticles));
  }
}

void ScriptInterface::leaderelectiondemo() {
  sim.setSystem(LeaderElectionDemo::LeaderElectionDemo::instance());
}

void ScriptInterface::ring(const int numParticles, const float holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<RingSystem>(numParticles, holeProb));
  }
}

void ScriptInterface::universalcoating(const int staticParticlesRadius,
                                       const int numParticles,
                                       const float holeProb) {
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
