#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include <qdebug.h>
#include <QGuiApplication>

#include "sim/simulator.h"
#include "ui/commandhistorymanager.h"

class QQmlApplicationEngine;

class Application : public QGuiApplication
{
    Q_OBJECT
public:
    explicit Application(int argc, char *argv[]);
    ~Application();

protected:
    std::shared_ptr<QQmlApplicationEngine> engine;

    Simulator sim;

    CommandHistoryManager commandHistoryManager;
};

#endif // APPLICATION_H
