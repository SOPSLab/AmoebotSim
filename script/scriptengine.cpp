/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "script/scriptengine.h"

#include <QFile>
#include <QString>
#include <QTextStream>

#include "script/scriptinterface.h"

ScriptEngine::ScriptEngine(Simulator& sim, VisItem* vis)
  : scriptInterface(new ScriptInterface(*this, sim, vis)) {
  // Create a global object for the JavaScript engine and make its methods
  // globally accessible. The engine owns the script interface.
  auto globalObject = engine.newQObject(scriptInterface);
  engine.globalObject().setProperty("globalObject", globalObject);
  engine.evaluate("Object.keys(globalObject).forEach(function(key){ this[key] = globalObject[key] })");
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
