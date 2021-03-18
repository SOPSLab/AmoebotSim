/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "script/scriptengine.h"

#include <QFile>
#include <QString>
#include <QTextStream>

#include "script/scriptinterface.h"

ScriptEngine::ScriptEngine(Simulator& sim, VisItem* vis, AlgorithmList* algList)
  : scriptInterface(new ScriptInterface(*this, sim, vis)),
    _algList(algList) {
  // Create a global object for the JavaScript engine and make its methods
  // globally accessible. The engine owns the script interface.
  auto globalObject = engine.newQObject(scriptInterface);
  engine.globalObject().setProperty("globalObject", globalObject);
  engine.evaluate("Object.keys(globalObject).forEach(function(key){ this[key] = globalObject[key] })");

  // For each algorithm, register it with the script engine and associate its
  // signature (e.g., 'shapeformation' for the Basic Shape Formation algorithm)
  // with its ::instantiate() function defined in ui/algorithm.*
  for (auto alg : _algList->getAlgs()) {
    auto algObject = engine.newQObject(alg);
    engine.globalObject().setProperty(alg->getSignature(), algObject);
    engine.evaluate("this[\"" + alg->getSignature() + "\"] = " + alg->getSignature() + "[\"instantiate\"]");
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
