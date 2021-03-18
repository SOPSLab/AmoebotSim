/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "script/scriptinterface.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

#include "alg/shapeformation.h"
#include "core/node.h"

ScriptInterface::ScriptInterface(ScriptEngine &engine, Simulator& sim,
                                 VisItem *vis)
  : engine(engine),
    sim(sim),
    vis(vis) {
  sim.setSystem(std::make_shared<ShapeFormationSystem>(200, 0.2, "h"));
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

void ScriptInterface::setStepDuration(const int ms) {
  if (ms < 0) {
    log("Step duration must be non-negative", true);
    sim.setStepDuration(0);
  } else {
    sim.setStepDuration(ms);
  }
}

void ScriptInterface::runUntilTermination() {
  sim.runUntilTermination();
}

int ScriptInterface::getNumParticles() {
  return sim.numParticles();
}

int ScriptInterface::getNumObjects() {
  return sim.numObjects();
}

void ScriptInterface::exportMetrics() {
  sim.exportMetrics();
  log("Metrics exported to application directory.");
}

QVariant ScriptInterface::getMetric(QString name, bool history) {
  for (const auto& c : sim.getSystem()->getCounts()) {
    if (c->_name == name) {
      return history ? QVariant::fromValue(c->_history) : c->_value;
    }
  }
  for (const auto& m : sim.getSystem()->getMeasures()) {
    if (m->_name == name) {
      return history ? QVariant::fromValue(m->_history) : m->_history.back();
    }
  }
  log("no metrics with given name exist", true);
  return QVariant();
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
               QString::number(QDateTime::currentSecsSinceEpoch()) + ".png";
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

QString ScriptInterface::pad(const int number, const int length) {
  QString str = "" + QString::number(number);

  while(str.length() < length) {
    str = QString("0") + str;
  }

  return str;
}
