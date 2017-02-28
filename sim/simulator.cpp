#include <QMutexLocker>

#include "script/scriptinterface.h"
#include "sim/simulator.h"
#include "alg/legacy/legacysystem.h"
Simulator::Simulator()
{
    roundTimer.setInterval(100);
    connect(&roundTimer, &QTimer::timeout, this, &Simulator::round);
}

Simulator::~Simulator()
{
    roundTimer.stop();
}

void Simulator::setSystem(std::shared_ptr<System> _system)
{
    roundTimer.stop();
    emit stopped();

    system = _system;
    emit systemChanged(system);
}

std::shared_ptr<System> Simulator::getSystem() const
{
    return system;
}

void Simulator::round()
{
    QMutexLocker locker(&system->mutex);
    system->activate();

    if(system->hasTerminated()) {
        stop();
    }
}

void Simulator::roundForParticleAt(Node node)
{
    QMutexLocker locker(&system->mutex);
    system->activateParticleAt(node);
}

void Simulator::runUntilTermination()
{
    QMutexLocker locker(&system->mutex);
    while(!system->hasTerminated()) {
        system->activate();
    }
}

void Simulator::start()
{
    roundTimer.start();
    emit started();
}

void Simulator::stop()
{
    roundTimer.stop();
    emit stopped();
}

void Simulator::saveScreenshotSetup(const QString filePath)
{
    emit systemChanged(system);
    emit saveScreenshot(filePath);
}

int Simulator::numParticles() const
{
    QMutexLocker locker(&system->mutex);
    return system->size();
}

int Simulator::numMovements() const
{
    QMutexLocker locker(&system->mutex);
    return system->numMovements();
}

int Simulator::numRounds() const
{
    QMutexLocker locker(&system->mutex);
    return system->numRounds();
}
int Simulator::leaderElectionRounds() const
{
    QMutexLocker locker(&system->mutex);
    return system->leaderElectionRounds();
}
int Simulator::weakBounds() const
{
    QMutexLocker locker(&system->mutex);
    return system->weakBounds();
}
int Simulator::strongBounds() const
{
    QMutexLocker locker(&system->mutex);
    return system->strongBounds();
}
void Simulator::setRoundDuration(int ms)
{
    roundTimer.setInterval(ms);
    emit roundDurationChanged(ms);
}
