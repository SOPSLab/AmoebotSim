#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "script/scriptengine.h"
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
    std::shared_ptr<ScriptEngine> scriptEngine;
};

#endif // APPLICATION_H
