/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "script/scriptengine.h"

#include <QFile>
#include <QString>
#include <QTextStream>

#include "script/scriptinterface.h"

ScriptEngine::ScriptEngine(Simulator& sim, VisItem* vis, AlgorithmList* list)
  : scriptInterface(new ScriptInterface(*this, sim, vis)) {
  // Create a global object for the JavaScript engine and make its methods
  // globally accessible. The engine owns the script interface.
  algList = list;
  auto globalObject = engine.newQObject(scriptInterface);
  engine.globalObject().setProperty("globalObject", globalObject);
  engine.evaluate("Object.keys(globalObject).forEach(function(key){ this[key] = globalObject[key] })");

  for (auto alg : algList->getAlgs()) {
    auto algObject = engine.newQObject(alg);
    engine.globalObject().setProperty(alg->getSignature(), algObject);
    // Create a string that will register each algorithm's instantiate function
    // as a global function under its signature name, e.g., for Shape Formation
    // the JS function to instantiate it is
    // 'shapeformation(numParticles, holeProb, mode)'
    QString JScmd = "this[" + alg->getSignature() + "] = " + alg->getSignature() + "[instantiate]";
    engine.evaluate(JScmd);
  }
}

void ScriptEngine::executeCommand(const QString cmd) {
  auto result = engine.evaluate(cmd);
  if (!result.isUndefined()) {
    emit log(result.toString(), result.isError());
  }
}

void ScriptEngine::runScript(const QString scriptFilePath) {
  QFile scriptFile(scriptFilePath);

  if (!scriptFile.open(QFile::ReadOnly)) {
    emit log("could not open script file", true);
    return;
  }

  QTextStream stream(&scriptFile);
  const QString script = stream.readAll();

  scriptFile.close();

  engine.evaluate(script);
}
