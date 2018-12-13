#include <memory>

#include <QDebug>
#include <QQmlContext>
#include <QTimer>

#include "main/application.h"
#include "ui/visitem.h"

// for evaluation, you can specify the path to a script here
// the simulator is then started without the GUI, executes the scripts, and terminates once the execution is complete
// messages that would be logged to the GUI are instead logged to qDebug()
static const QString scriptPath = "";

Application::Application(int argc, char *argv[])
  : QGuiApplication(argc, argv) {
  if (scriptPath == "") {
    // Setup GUI.
    qmlRegisterType<VisItem>("VisItem", 1, 0, "VisItem");
    engine.rootContext()->setContextProperty("parameterModel", &parameterModel);
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));
    auto qmlRoot = engine.rootObjects().first();
    auto vis = qmlRoot->findChild<VisItem*>();
    auto slider = qmlRoot->findChild<QObject*>("stepDurationSlider");
    connect(vis, &VisItem::beforeRendering,
            [this, qmlRoot](){
                QMetaObject::invokeMethod(qmlRoot, "setNumMovements", Q_ARG(QVariant, sim.numMovements()));
            }
    );
    connect(vis, &VisItem::beforeRendering,
            [this, qmlRoot](){
                QMetaObject::invokeMethod(qmlRoot, "setNumRounds", Q_ARG(QVariant, sim.numRounds()));
            }
    );
    connect(vis, &VisItem::inspectParticle,
            [qmlRoot](QString text){
                QMetaObject::invokeMethod(qmlRoot, "inspectParticle", Q_ARG(QVariant, text));
            }
    );

    // Populate algorithm selection combo box with algorithm names.
    auto algbox = qmlRoot->findChild<QObject*>("algorithmSelectBox");
    algbox->setProperty("model", QVariant::fromValue(algs.getAlgNames()));

    // Set the parameter list model's algorithms and connect it to the signals
    // from other UI elements.
    parameterModel.setAlgorithmList(&algs);
    connect(qmlRoot, SIGNAL(algSelected(QString)),
            &parameterModel, SLOT(updateAlgParameters(QString)));
    connect(qmlRoot, SIGNAL(instantiate(QString)),
            &parameterModel, SLOT(createCommand(QString)));

    // setup connections between GUI and Simulator
    connect(&sim, &Simulator::systemChanged, vis, &VisItem::systemChanged);
    connect(&sim, &Simulator::saveScreenshot, vis, &VisItem::saveScreenshot);
    connect(qmlRoot, SIGNAL(start()), &sim, SLOT(start()));
    connect(qmlRoot, SIGNAL(stop()), &sim, SLOT(stop()));
    connect(qmlRoot, SIGNAL(step()), &sim, SLOT(step()));
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
    connect(&parameterModel, SIGNAL(executeCommand(QString)), scriptEngine.get(), SLOT(executeCommand(QString)));

    // Set default step duration.
    sim.setStepDuration(0);
  } else {
    scriptEngine = std::make_shared<ScriptEngine>(sim, nullptr);
    connect(scriptEngine.get(),  &ScriptEngine::log, [](const QString msg, const bool isError){ Q_UNUSED(isError); qDebug() << msg; });
    scriptEngine->executeCommand("runScript(\"" + scriptPath + "\")");
    // create one shot timer that quits the application once the script is executed
    // this has to be done in this convoluted way since quit does not seem to have any effect until the event loop runs
    QTimer* quitTimer = new QTimer(this);
    quitTimer->start(0);
    connect(quitTimer, &QTimer::timeout, this, &QCoreApplication::quit);
  }
}
