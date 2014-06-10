#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QScriptEngine>
#include <QTimer>

#include "system.h"

class QTimer;

class Simulator : public QObject
{
    Q_OBJECT

    friend class ScriptInterface;

public:
    explicit Simulator();

    void registerAlgorithm(QString name, void (*fn)());

signals:
    void updateSystem(System* _system);

    void started();
    void stopped();

    void log(const QString msg, const bool isError);

public slots:
    void init();
    void round();
    void start();
    void stop();

    void executeCommand(const QString cmd);

protected:
    QScriptEngine engine;
    QTimer* roundTimer;

    System system;
};

#endif // SIMULATOR_H
