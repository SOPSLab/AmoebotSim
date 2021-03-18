/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#ifndef AMOEBOTSIM_MAIN_APPLICATION_H_
#define AMOEBOTSIM_MAIN_APPLICATION_H_

#include <memory>

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "core/simulator.h"
#include "script/scriptengine.h"
#include "ui/parameterlistmodel.h"

class Application : public QGuiApplication {
  Q_OBJECT
 public:
  explicit Application(int argc, char *argv[]);

 protected:
  QQmlApplicationEngine engine;
  Simulator sim;
  std::shared_ptr<ScriptEngine> scriptEngine;
  ParameterListModel* parameterModel;
};

#endif  // AMOEBOTSIM_MAIN_APPLICATION_H_
