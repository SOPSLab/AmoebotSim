#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <memory>

#include <QJSEngine>

#include "system.h"

class QTimer;

class Simulator : public QObject
{
    Q_OBJECT

public:
    explicit Simulator();
    virtual ~Simulator();

    void setSystem(std::shared_ptr<System> _system);
    std::shared_ptr<System> getSystem() const;

signals:
    void systemChanged(std::shared_ptr<System> _system);
    void roundDurationChanged(int ms);
    void numMovementsChanged(int num);
    void numRoundsChanged(int num);

    void started();
    void stopped();

    void log(const QString msg, const bool isError);

public slots:
    void init();
    void round();
    void start();
    void stop();

    void runUntilNonValid();

    void finished();

    void roundForParticleAt(const int x, const int y);

    void executeCommand(const QString cmd);
    void runScript(const QString script);
    void abortScript();

    bool getSystemValid();
    bool getSystemDisconnected();
    bool getSystemTerminated();
    bool getSystemDeadlocked();

    int getNumParticles() const;
    int getNumNonStaticParticles() const;
    int getNumMovements() const;
    int getNumRounds() const;

    void setCheckConnectivity(bool b);
    void setRoundDuration(int ms);

protected:
    QJSEngine engine;

    std::shared_ptr<QTimer> roundTimer;

    std::shared_ptr<System> system;
};

#endif // SIMULATOR_H
