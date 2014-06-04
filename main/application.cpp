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

    // setup connections between gui and simulation
    VisItem* vis = engine->rootObjects().at(0)->findChild<VisItem*>();
    connect(sim, &Simulator::updateSystem, vis, &VisItem::updateSystem);
    connect(engine->rootObjects().at(0), SIGNAL(start()), sim, SLOT(start()));
    connect(engine->rootObjects().at(0), SIGNAL(stop() ), sim, SLOT(stop() ));
    connect(engine->rootObjects().at(0), SIGNAL(round()), sim, SLOT(round()));

    connect(simThread, SIGNAL(started()), sim, SLOT(init()));
    simThread->start();
}

Application::~Application()
{
    simThread->quit();
    simThread->wait();
    delete sim;
    delete engine;
}
