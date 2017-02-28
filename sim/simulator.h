#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <memory>

#include <QObject>
#include <QTimer>

#include "sim/system.h"

class Simulator : public QObject
{
    Q_OBJECT
public:
    Simulator();
    virtual ~Simulator();

    void setSystem(std::shared_ptr<System> _system);
    std::shared_ptr<System> getSystem() const;

signals:
    void systemChanged(std::shared_ptr<System> _system);
    void roundDurationChanged(int ms);
    void saveScreenshot(const QString filePath);

    void started();
    void stopped();

public slots:
    void round();
    void roundForParticleAt(Node node);
    void runUntilTermination();
    void start();
    void stop();
    void saveScreenshotSetup(const QString filePath);

    int numParticles() const;
    int numMovements() const;
    int numRounds() const;
    int leaderElectionRounds() const;
    int weakBounds() const;
    int strongBounds() const;
    void setRoundDuration(int ms);

protected:
    QTimer roundTimer;

    std::shared_ptr<System> system;
};

#endif // SIMULATOR_H
