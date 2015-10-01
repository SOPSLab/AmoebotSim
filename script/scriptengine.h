#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QJSEngine>
#include <QObject>

#include "sim/simulator.h"

#include "ui/visitem.h"

class ScriptInterface;

class ScriptEngine : public QObject
{
    Q_OBJECT
public:
    ScriptEngine(Simulator& sim, VisItem* vis = nullptr);

signals:
    void log(const QString msg, bool error = false);

public slots:
    void executeCommand(const QString cmd);
    void runScript(const QString scriptFilePath);

private:
    QJSEngine engine;
    ScriptInterface* scriptInterface;
};

#endif // SCRIPTENGINE_H
