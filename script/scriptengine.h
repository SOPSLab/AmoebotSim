/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#ifndef AMOEBOTSIM_SCRIPT_SCRIPTENGINE_H_
#define AMOEBOTSIM_SCRIPT_SCRIPTENGINE_H_

#include <QJSEngine>
#include <QObject>

#include "core/simulator.h"
#include "ui/visitem.h"

// ScriptInterface must be forward declared to avoid a dependency loop.
class ScriptInterface;

class ScriptEngine : public QObject {
  Q_OBJECT

 public:
  ScriptEngine(Simulator& sim, VisItem* vis = nullptr);

 signals:
  void log(const QString msg, bool error = false);

 public slots:
  void executeCommand(const QString cmd);
  void runScript(const QString scriptFilePath);

 private:
  QJSEngine engine;
  ScriptInterface* scriptInterface;
};

#endif  // AMOEBOTSIM_SCRIPT_SCRIPTENGINE_H_
