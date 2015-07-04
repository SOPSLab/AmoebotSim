#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>

#include <QGuiApplication>

#include "ui/commandhistorymanager.h"

class QQmlApplicationEngine;
class QThread;

class Simulator;

class Application : public QGuiApplication
{
    Q_OBJECT
public:
    explicit Application(int argc, char *argv[]);
    ~Application();

protected:
    std::shared_ptr<QQmlApplicationEngine> engine;

    std::shared_ptr<Simulator> sim;
    std::shared_ptr<QThread> simThread;

    CommandHistoryManager commandHistoryManager;
};

#endif // APPLICATION_H
