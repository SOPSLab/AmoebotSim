#include <QQmlApplicationEngine>
#include <QThread>

#include "main/application.h"
#include "sim/simulator.h"
#include "ui/visitem.h"

Application::Application(int argc, char *argv[]) :
    QGuiApplication(argc, argv),
    engine(new QQmlApplicationEngine()),
    sim(new Simulator()),
    simThread(new QThread(this))
{
    qmlRegisterType<VisItem>("VisItem", 1, 0, "VisItem");
    engine->load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    sim->moveToThread(simThread);

    // setup connections between GUI and Simulator
    VisItem* vis = engine->rootObjects().at(0)->findChild<VisItem*>();
    connect(sim, &Simulator::updateSystem, vis, &VisItem::updateSystem);
    connect(sim, &Simulator::saveScreenshotSignal, vis, &VisItem::saveScreenshot);
    connect(engine->rootObjects().at(0), SIGNAL(start()), sim, SLOT(start()));
    connect(engine->rootObjects().at(0), SIGNAL(stop() ), sim, SLOT(stop() ));
    connect(engine->rootObjects().at(0), SIGNAL(round()), sim, SLOT(round()));
    connect(vis, &VisItem::roundForParticleAt, sim, &Simulator::roundForParticleAt);
    connect(engine->rootObjects().at(0), SIGNAL(executeCommand(QString)), sim, SLOT(executeCommand(QString)));
    connect(sim,
            &Simulator::log,
            [&](const QString msg, const bool isError){
                QMetaObject::invokeMethod(engine->rootObjects().at(0), "log", Q_ARG(QVariant, msg), Q_ARG(QVariant, isError));
            }
    );
    connect(sim,
            &Simulator::started,
            [&](){
                QMetaObject::invokeMethod(engine->rootObjects().at(0), "setLabelStop");
            }
    );
    connect(sim,
            &Simulator::stopped,
            [&](){
                QMetaObject::invokeMethod(engine->rootObjects().at(0), "setLabelStart");
            }
    );

    // setup connections between GUI and CommmandHistoryManager
    connect(engine->rootObjects().at(0), SIGNAL(executeCommand(QString)), &commandHistoryManager, SLOT(commandExecuted(QString)));
    connect(engine->rootObjects().at(0), SIGNAL(commandFieldUp()), &commandHistoryManager, SLOT(up()));
    connect(engine->rootObjects().at(0), SIGNAL(commandFieldDown()), &commandHistoryManager, SLOT(down()));
    connect(engine->rootObjects().at(0), SIGNAL(commandFieldReset()), &commandHistoryManager, SLOT(reset()));
    connect(&commandHistoryManager,
            &CommandHistoryManager::setCommand,
            [&](const QString& command){
                QMetaObject::invokeMethod(engine->rootObjects().at(0), "setCommand", Q_ARG(QVariant, command));
            }
    );

    connect(simThread, &QThread::started, sim, &Simulator::init);
    connect(simThread, &QThread::finished, sim, &Simulator::finished);

    simThread->start();
}

Application::~Application()
{
    sim->abortScript();
    simThread->quit();
    simThread->wait();
    delete sim;
    delete engine;
}
