#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QScriptEngine>

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
    void saveScreenshotSignal(const QString filePath);

public slots:
    void init();
    void round();
    void start();
    void stop();

    void roundForParticleAt(const int x, const int y);

    void executeCommand(const QString cmd);
    void runScript(const QString script);
    void abortScript();

    bool getSystemValid();
    bool getSystemDisconnected();
    bool getSystemTerminated();
    bool getSystemDeadlocked();

    int getNumParticles() const;
    int getNumMovements() const;

    void setRoundDuration(int ms);
    void saveScreenshotSlot(const QString filePath);

protected:
    QScriptEngine engine;
    QTimer* roundTimer;
    QTimer* updateTimer;

    System* system;
};

#endif // SIMULATOR_H
