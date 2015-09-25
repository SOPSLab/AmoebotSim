#include <memory>

#include "main/application.h"
#include "ui/visitem.h"

Application::Application(int argc, char *argv[]) :
    QGuiApplication(argc, argv)
{
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

    connect(slider, SIGNAL(roundDurationChanged(int)), &sim, SLOT(setRoundDuration(int)));
    connect(&sim, &Simulator::roundDurationChanged,
            [slider](const int& ms){
              QMetaObject::invokeMethod(slider, "setRoundDuration", Q_ARG(QVariant, QVariant(ms)));
            }
    );

//    connect(&sim,  &Simulator::numMovementsChanged,
//            [qmlRoot](const int& num){
//                QMetaObject::invokeMethod(qmlRoot, "setNumMovements", Q_ARG(QVariant, num));
//            }
//    );
//    connect(&sim,  &Simulator::numRoundsChanged,
//            [qmlRoot](const int& rounds){
//                QMetaObject::invokeMethod(qmlRoot, "setNumRounds", Q_ARG(QVariant, rounds));
//            }
//    );
    connect(&sim,  &Simulator::log,
            [qmlRoot](const QString msg, const bool isError){
                QMetaObject::invokeMethod(qmlRoot, "log", Q_ARG(QVariant, msg), Q_ARG(QVariant, isError));
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

    sim.emitInitialSignals();
}
