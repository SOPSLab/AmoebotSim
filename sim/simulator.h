#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <memory>

#include <QJSEngine>
#include <QTimer>

#include "system.h"

class Simulator : public QObject
{
    Q_OBJECT

public:
    explicit Simulator();
    virtual ~Simulator();

    void setSystem(std::shared_ptr<System> _system);
    std::shared_ptr<System> getSystem() const;

signals:
    void systemChanged(std::shared_ptr<System>& _system);
    void roundDurationChanged(int ms);

    void started();
    void stopped();

    void log(const QString msg, const bool isError);

public slots:
    void init();
    void round();
    void start();
    void stop();

    void finished();

    void executeCommand(const QString cmd);
    void runScript(const QString script);
    void abortScript();

    int getNumParticles() const;
    int getNumMovements() const;
    int getNumRounds() const;

    void setRoundDuration(int ms);

protected:
    QJSEngine engine;

    QTimer roundTimer;

    std::shared_ptr<System> system;
};

#endif // SIMULATOR_H
