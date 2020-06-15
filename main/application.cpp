/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "main/application.h"

#include <QDebug>
#include <QQmlContext>
#include <QString>
#include <QStringList>

#include "ui/visitem.h"

Application::Application(int argc, char *argv[])
    : QGuiApplication(argc, argv) {
  // Setup the parameter list model.
  parameterModel = new ParameterListModel();
  engine.rootContext()->setContextProperty("parameterModel", parameterModel);

  // Setup GUI.
  qmlRegisterType<VisItem>("VisItem", 1, 0, "VisItem");
  engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));
  auto qmlRoot = engine.rootObjects().first();
  auto vis = qmlRoot->findChild<VisItem*>();
  auto slider = qmlRoot->findChild<QObject*>("stepDurationSlider");
  connect(vis, &VisItem::beforeRendering,
          [this, qmlRoot](){
            QMetaObject::invokeMethod(qmlRoot, "setMetrics", Q_ARG(QVariant, sim.metrics()));
          }
  );
  connect(vis, &VisItem::inspectParticle,
          [qmlRoot](QString text){
            QMetaObject::invokeMethod(qmlRoot, "inspectParticle", Q_ARG(QVariant, text));
          }
  );

  // Populate algorithm selection combo box with algorithm names and set its
  // initial value.
  auto algBox = qmlRoot->findChild<QObject*>("algorithmSelectBox");
  QStringList names = parameterModel->getAlgorithmList()->getAlgNames();
  algBox->setProperty("model", QVariant::fromValue(names));
  algBox->setProperty("currentIndex", names.indexOf("Basic Shape Formation"));
  parameterModel->updateAlgParameters("Basic Shape Formation");

  // Connect the parameter list model to the UI elements that use it.
  connect(qmlRoot, SIGNAL(algSelected(QString)),
          parameterModel, SLOT(updateAlgParameters(QString)));
  connect(qmlRoot, SIGNAL(instantiate(QString)),
          parameterModel, SLOT(createSystem(QString)));
  for (Algorithm* alg : parameterModel->getAlgorithmList()->getAlgs()) {
    connect(alg, &Algorithm::log, [qmlRoot](const QString msg, const bool isError){
      QMetaObject::invokeMethod(qmlRoot, "log", Q_ARG(QVariant, msg), Q_ARG(QVariant, isError));
    });
    connect(alg, &Algorithm::setSystem, &sim, &Simulator::setSystem);
  }

  // setup connections between GUI and Simulator
  connect(&sim, &Simulator::systemChanged, vis, &VisItem::systemChanged);
  connect(&sim, &Simulator::saveScreenshot, vis, &VisItem::saveScreenshot);
  connect(qmlRoot, SIGNAL(start()), &sim, SLOT(start()));
  connect(qmlRoot, SIGNAL(stop()), &sim, SLOT(stop()));
  connect(qmlRoot, SIGNAL(step()), &sim, SLOT(step()));
  connect(qmlRoot, SIGNAL(exportMetrics()), &sim, SLOT(exportMetrics()));
  connect(&sim, &Simulator::started,
          [qmlRoot](){
            QMetaObject::invokeMethod(qmlRoot, "setLabelStop");
          }
  );
  connect(&sim, &Simulator::stopped,
          [qmlRoot](){
            QMetaObject::invokeMethod(qmlRoot, "setLabelStart");
          }
  );
  connect(vis, &VisItem::stepForParticleAt, &sim, &Simulator::stepForParticleAt);
  connect(slider, SIGNAL(stepDurationChanged(int)), &sim, SLOT(setStepDuration(int)));
  connect(&sim, &Simulator::stepDurationChanged,
          [slider](const int& ms){
            QMetaObject::invokeMethod(slider, "setStepDuration", Q_ARG(QVariant, QVariant(ms)));
          }
  );

  // setup scripting
  scriptEngine = std::make_shared<ScriptEngine>(sim, vis, parameterModel->getAlgorithmList());
  connect(scriptEngine.get(), &ScriptEngine::log,
          [qmlRoot](const QString msg, const bool isError){
            QMetaObject::invokeMethod(qmlRoot, "log", Q_ARG(QVariant, msg), Q_ARG(QVariant, isError));
          }
  );

  // Set default step duration.
  sim.setStepDuration(0);
}
