#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QScriptEngine>
#include <QTimer>

#include "system.h"

class QTimer;

class Simulator : public QObject
{
    Q_OBJECT

public:
    explicit Simulator();
    virtual ~Simulator();

    void setSystem(System* _system);

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

    System* system;
};

#endif // SIMULATOR_H
