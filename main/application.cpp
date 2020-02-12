/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "main/application.h"

#include <QDebug>
#include <QQmlContext>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "ui/visitem.h"

// for evaluation, you can specify the path to a script here
// the simulator is then started without the GUI, executes the scripts, and terminates once the execution is complete
// messages that would be logged to the GUI are instead logged to qDebug()
static const QString scriptPath = "";

Application::Application(int argc, char *argv[])
    : QGuiApplication(argc, argv) {
  if (scriptPath == "") {
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
            parameterModel, SLOT(createCommand(QString)));

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

    // setup connections between GUI and CommmandHistoryManager
    connect(qmlRoot, SIGNAL(executeCommand(QString)), &commandHistoryManager, SLOT(commandExecuted(QString)));
    connect(qmlRoot, SIGNAL(commandFieldUp()), &commandHistoryManager, SLOT(up()));
    connect(qmlRoot, SIGNAL(commandFieldDown()), &commandHistoryManager, SLOT(down()));
    connect(qmlRoot, SIGNAL(commandFieldReset()), &commandHistoryManager, SLOT(reset()));
    connect(&commandHistoryManager, &CommandHistoryManager::setCommand,
            [qmlRoot](const QString& command){
              QMetaObject::invokeMethod(qmlRoot, "setCommand", Q_ARG(QVariant, command));
            }
    );

    // setup scripting
    scriptEngine = std::make_shared<ScriptEngine>(sim, vis);
    connect(scriptEngine.get(), &ScriptEngine::log,
            [qmlRoot](const QString msg, const bool isError){
              QMetaObject::invokeMethod(qmlRoot, "log", Q_ARG(QVariant, msg), Q_ARG(QVariant, isError));
            }
    );
    connect(qmlRoot, SIGNAL(executeCommand(QString)), scriptEngine.get(), SLOT(executeCommand(QString)));
    connect(parameterModel, SIGNAL(executeCommand(QString)), scriptEngine.get(), SLOT(executeCommand(QString)));

    // Set default step duration.
    sim.setStepDuration(0);
  } else {
    scriptEngine = std::make_shared<ScriptEngine>(sim, nullptr);
    connect(scriptEngine.get(), &ScriptEngine::log, [](const QString msg, const bool isError){ Q_UNUSED(isError); qDebug() << msg; });
    scriptEngine->executeCommand("runScript(\"" + scriptPath + "\")");
    // create one shot timer that quits the application once the script is executed
    // this has to be done in this convoluted way since quit does not seem to have any effect until the event loop runs
    QTimer* quitTimer = new QTimer(this);
    quitTimer->start(0);
    connect(quitTimer, &QTimer::timeout, this, &QCoreApplication::quit);
  }
}
