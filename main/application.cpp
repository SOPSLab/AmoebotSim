#include <QDebug>
#include <QTimer>

#include "main/application.h"
#include "ui/visitem.h"

// for evaluation, you can specify the path to a script here
// the simulator is then started without the GUI, executes the scripts, and terminates once the execution is complete
// messages that would be logged to the GUI are instead logged to qDebug()
static const QString scriptPath = "";

Application::Application(int argc, char *argv[])
    : QGuiApplication(argc, argv)
{
    if(scriptPath == "") {
        qmlRegisterType<VisItem>("VisItem", 1, 0, "VisItem");
        engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));
        auto qmlRoot = engine.rootObjects().first();

        // setup connections between GUI and Simulator
        auto vis = qmlRoot->findChild<VisItem*>();
        auto slider = qmlRoot->findChild<QObject*>("roundDurationSlider");

        connect(&sim, &Simulator::systemChanged, vis, &VisItem::systemChanged);
        connect(qmlRoot, SIGNAL(start()), &sim, SLOT(start()));
        connect(qmlRoot, SIGNAL(stop() ), &sim, SLOT(stop() ));
        connect(qmlRoot, SIGNAL(round()), &sim, SLOT(round()));
        connect(qmlRoot, SIGNAL(executeCommand(QString)), &sim, SLOT(executeCommand(QString)));

        connect(vis, &VisItem::roundForParticleAt, &sim, &Simulator::roundForParticleAt);

        connect(slider, SIGNAL(roundDurationChanged(int)), &sim, SLOT(setRoundDuration(int)));
        connect(&sim, &Simulator::roundDurationChanged,
                [slider](const int& ms){
                  QMetaObject::invokeMethod(slider, "setRoundDuration", Q_ARG(QVariant, QVariant(ms)));
                }
        );
        connect(vis,  &VisItem::beforeRendering,
                [this, qmlRoot](){
                    QMetaObject::invokeMethod(qmlRoot, "setNumMovements", Q_ARG(QVariant, sim.numMovements()));
                }
        );
        connect(vis,  &VisItem::beforeRendering,
                [this, qmlRoot](){
                    QMetaObject::invokeMethod(qmlRoot, "setNumRounds", Q_ARG(QVariant, sim.numRounds()));
                }
        );
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
        connect(&sim,  &Simulator::log,
                [qmlRoot](const QString msg, const bool isError){
                    QMetaObject::invokeMethod(qmlRoot, "log", Q_ARG(QVariant, msg), Q_ARG(QVariant, isError));
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

        // let the simulator inform the visualization about the current system etc.
        sim.emitInitialSignals();
    } else {
        connect(&sim,  &Simulator::log, [](const QString msg, const bool isError){ Q_UNUSED(isError); qDebug() << msg; });
        sim.executeCommand("runScript(\"" + scriptPath + "\")");
        // create one shot timer that quits the application once the script is executed
        // this has to be done in this convoluted way since quit does not seem to have any effect until the event loop runs
        QTimer* quitTimer = new QTimer(this);
        quitTimer->start(0);
        connect(quitTimer, &QTimer::timeout, this, &QCoreApplication::quit);
    }
}
