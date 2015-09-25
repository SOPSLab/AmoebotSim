#ifndef APPLICATION_H
#define APPLICATION_H

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "sim/simulator.h"
#include "ui/commandhistorymanager.h"

class Application : public QGuiApplication
{
    Q_OBJECT
public:
    explicit Application(int argc, char *argv[]);

protected:
    QQmlApplicationEngine engine;

    Simulator sim;

    CommandHistoryManager commandHistoryManager;
};

#endif // APPLICATION_H
