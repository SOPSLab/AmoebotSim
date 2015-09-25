#include <QMutexLocker>

#include "script/scriptinterface.h"
#include "sim/simulator.h"

Simulator::Simulator()
{
    // Create a global object for the javascript engine and make its methods globally accessible.
    // Ownership of ScriptInterface-object is handled by QObject.
    auto globalObject = engine.newQObject(new ScriptInterface(*this));
    engine.globalObject().setProperty("globalObject", globalObject);
    engine.evaluate("Object.keys(globalObject).forEach(function(key){ this[key] = globalObject[key] })");

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

void Simulator::emitInitialSignals()
{
    emit roundDurationChanged(roundTimer.interval());
    emit systemChanged(system);
}

void Simulator::round()
{
    QMutexLocker locker(&system->mutex);
    system->activate();

    if(system->hasTerminated()) {
        stop();
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

void Simulator::executeCommand(const QString cmd)
{
    auto result = engine.evaluate(cmd);
    if(!result.isUndefined()) {
        emit log(result.toString(), result.isError());
    }
}

void Simulator::runScript(const QString script)
{
    engine.evaluate(script);
}

int Simulator::getNumParticles() const
{
    QMutexLocker locker(&system->mutex);
    return system->size();
}

int Simulator::getNumMovements() const
{
    QMutexLocker locker(&system->mutex);
    return system->numMovements();
}

int Simulator::getNumRounds() const
{
    QMutexLocker locker(&system->mutex);
    return system->numRounds();
}

void Simulator::setRoundDuration(int ms)
{
    roundTimer.setInterval(ms);
    emit roundDurationChanged(ms);
}
