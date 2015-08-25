#include <QQmlApplicationEngine>
#include <QThread>

#include "main/application.h"
#include "sim/simulator.h"
#include "ui/visitem.h"
#include <iostream>

Q_DECLARE_METATYPE(std::shared_ptr<System>)

Application::Application(int argc, char *argv[]) :
    QGuiApplication(argc, argv),
    engine(std::make_shared<QQmlApplicationEngine>()),
    sim(std::make_shared<Simulator>()),
    simThread(std::make_shared<QThread>(this))
{

    //register shared_ptr<System> to allow using it as a parameter in signals/slots
    qRegisterMetaType<std::shared_ptr<System>>();

    qmlRegisterType<VisItem>("VisItem", 1, 0, "VisItem");
    engine->load(QUrl(QStringLiteral("qrc:///qml/main.qml")));
    auto qmlRoot = engine->rootObjects().first();

    // setup connections between GUI and Simulator
    auto vis = qmlRoot->findChild<VisItem*>();
    auto slider = qmlRoot->findChild<QObject*>("roundDurationSlider");

    connect(sim.get(), &Simulator::updateSystem, vis, &VisItem::updateSystem);
    connect(sim.get(), &Simulator::moveCameraTo, vis, &VisItem::moveCameraTo);
    connect(sim.get(), &Simulator::setZoom, vis, &VisItem::setZoom);
    connect(sim.get(), &Simulator::saveScreenshotSignal, vis, &VisItem::saveScreenshot);
    connect(qmlRoot, SIGNAL(start()), sim.get(), SLOT(start()));
    connect(qmlRoot, SIGNAL(stop() ), sim.get(), SLOT(stop() ));
    connect(qmlRoot, SIGNAL(round()), sim.get(), SLOT(round()));
    connect(vis, &VisItem::roundForParticleAt, sim.get(), &Simulator::roundForParticleAt);
    connect(vis, &VisItem::insertParticleAt, sim.get(), &Simulator::insertParticleAt);
    connect(qmlRoot, SIGNAL(executeCommand(QString)), sim.get(), SLOT(executeCommand(QString)));

    connect(slider, SIGNAL(roundDurationChanged(int)), sim.get(), SLOT(setRoundDuration(int)));
    connect(sim.get(), &Simulator::roundDurationChanged,
            [slider](const int& ms){
              QMetaObject::invokeMethod(slider, "setRoundDuration", Q_ARG(QVariant, QVariant(ms)));
            }
    );

    connect(sim.get(),  &Simulator::numMovementsChanged,
            [qmlRoot](const int& num){
                QMetaObject::invokeMethod(qmlRoot, "setNumMovements", Q_ARG(QVariant, num));
            }
    );
    connect(sim.get(),  &Simulator::numRoundsChanged,
            [qmlRoot](const int& rounds){
                QMetaObject::invokeMethod(qmlRoot, "setNumRounds", Q_ARG(QVariant, rounds));
            }
    );
    connect(sim.get(),  &Simulator::log,
            [qmlRoot](const QString msg, const bool isError){
                QMetaObject::invokeMethod(qmlRoot, "log", Q_ARG(QVariant, msg), Q_ARG(QVariant, isError));
            }
    );
    connect(sim.get(), &Simulator::started,
            [qmlRoot](){
                QMetaObject::invokeMethod(qmlRoot, "setLabelStop");
            }
    );
    connect(sim.get(), &Simulator::stopped,
            [qmlRoot](){
                QMetaObject::invokeMethod(qmlRoot, "setLabelStart");
            }
    );
    connect(sim.get(), &Simulator::setResolution,
            [qmlRoot](const int width, const int height){
                QMetaObject::invokeMethod(qmlRoot, "setResolution", Q_ARG(QVariant, width), Q_ARG(QVariant, height));
            }
    );
    connect(sim.get(), &Simulator::setFullscreen,
            [qmlRoot](){
                QMetaObject::invokeMethod(qmlRoot, "setFullscreen");
            }
    );
    connect(sim.get(), &Simulator::setWindowed,
            [qmlRoot](){
                QMetaObject::invokeMethod(qmlRoot, "setWindowed");
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

    sim->moveToThread(simThread.get());
    connect(simThread.get(), &QThread::started, sim.get(), &Simulator::init);
    connect(simThread.get(), &QThread::finished, sim.get(), &Simulator::finished);


    simThread->start();
}

Application::~Application()
{
    sim->abortScript();
    simThread->quit();
    simThread->wait();
}
