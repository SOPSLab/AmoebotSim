#include <QMutexLocker>
#include <QTimer>

#include "script/scriptinterface.h"
#include "sim/simulator.h"

Simulator::Simulator()
{
    //It's not possible to set ScriptInterface's slots as global functions in the JS-Engine.
    //Therefore create a ScriptInterface object and bind it to the global variable 'obj'.
    auto obj = engine.newQObject(new ScriptInterface(*this));
    engine.globalObject().setProperty("obj", obj);

    //Then bind obj's methods (which are ScriptInterface's slots) to the global scope in 'this',
    //so that all methods are globally accessible.
    engine.evaluate("Object.keys(obj).forEach(function(key){ this[key] = obj[key]})");
}

Simulator::~Simulator()
{
}

void Simulator::setSystem(std::shared_ptr<System> _system)
{
    roundTimer.stop();
    emit stopped();

    system = _system;
    emit systemChanged(system);

    QMutexLocker locker(&system->mutex);
}

std::shared_ptr<System> Simulator::getSystem() const
{
    return system;
}

void Simulator::init()
{
    roundTimer.setInterval(100);
    emit roundDurationChanged(100);
    connect(&roundTimer, &QTimer::timeout, this, &Simulator::round);

    emit systemChanged(system);
}

//Is called when thread in which simulator is living is about to finish -> Clean up the simulator.
void Simulator::finished(){
    roundTimer.stop();
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

void Simulator::abortScript()
{
    //engine.abortEvaluation();
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
