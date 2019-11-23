#include <QFile>
#include <QTextStream>
#include <QTime>
#include <math.h>

#include "alg/demo/discodemo.h"
#include "alg/demo/pulldemo.h"
#include "alg/demo/tokendemo.h"
#include "alg/compression.h"
#include "alg/infobjcoating.h"
#include "alg/leaderelection.h"
#include "alg/shapeformation.h"

#include "script/scriptinterface.h"
#include "core/node.h"

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

void ScriptInterface::compression(const int numParticles, const double lambda) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else {
    sim.setSystem(std::make_shared<CompressionSystem>(numParticles, lambda));
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

void ScriptInterface::leaderelection(const int numParticles,
                                         const double holeProb) {
  if (numParticles <= 0) {
    log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    log("holeProb in [0,1] required", true);
  } else {
    sim.setSystem(std::make_shared<LeaderElectionSystem>(numParticles,
                                                         holeProb));
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

QString ScriptInterface::pad(const int number, const int length) {
  QString str = "" + QString::number(number);

  while(str.length() < length) {
    str = QString("0") + str;
  }

  return str;
}
