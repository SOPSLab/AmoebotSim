#ifndef APPLICATION_H
#define APPLICATION_H

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
    QQmlApplicationEngine* engine;

    Simulator* sim;
    QThread* simThread;

    CommandHistoryManager commandHistoryManager;
};

#endif // APPLICATION_H
