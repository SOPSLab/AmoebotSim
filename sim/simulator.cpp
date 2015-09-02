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
    if(roundTimer != nullptr) {
        roundTimer->stop();
        emit stopped();
    }
    system = _system;
    emit numMovementsChanged(system->getNumMovements());
    emit numRoundsChanged(system->getNumRounds());
}

void Simulator::init()
{
    if(roundTimer == nullptr) {
        roundTimer = std::make_shared<QTimer>(this);
        roundTimer->setInterval(100);
        emit roundDurationChanged(100);
        connect(roundTimer.get(), &QTimer::timeout, this, &Simulator::round);

        updateTimer = std::make_shared<QTimer>(this);
        updateTimer->setInterval(30);
        connect(updateTimer.get(), &QTimer::timeout, [&](){emit updateSystem(std::make_shared<System>(*system));});
        connect(updateTimer.get(), &QTimer::timeout, [&](){emit numMovementsChanged(system->getNumMovements());});
        connect(updateTimer.get(), &QTimer::timeout, [&](){emit numRoundsChanged(system->getNumRounds());});
        updateTimer->start();
    }
}

//Is called when thread in which simulator is living is about to finish -> Clean up the simulator.
void Simulator::finished(){
    roundTimer->stop();
    updateTimer->stop();
}

void Simulator::round()
{
    system->round();
    auto systemState = system->getSystemState();
    if(systemState == System::SystemState::Deadlocked) {
        log("Deadlock detected. Simulation aborted.", true);
        roundTimer->stop();
        emit stopped();
    } else if(systemState == System::SystemState::Disconnected) {
        log("System disconnected. Simulation aborted.", true);
        roundTimer->stop();
        emit stopped();
    } else if(systemState == System::SystemState::Terminated) {
        log("Algorithm terminated. Simulation finished.", false);
        roundTimer->stop();
        emit stopped();
    }
}

void Simulator::start()
{
    roundTimer->start();
    emit started();
}

void Simulator::stop()
{
    roundTimer->stop();
    emit stopped();
}

void Simulator::runUntilNonValid()
{
    system->runUntilNotValid();
}

void Simulator::roundForParticleAt(const int x, const int y)
{
    if(!roundTimer->isActive()) {
        const Node node(x, y);
        system->roundForParticle(node);
        auto systemState = system->getSystemState();
        if(systemState == System::SystemState::Deadlocked) {
            log("Deadlock detected.", true);
        } else if(systemState == System::SystemState::Disconnected) {
            log("System disconnected.", true);
        } else if(systemState == System::SystemState::Terminated) {
            log("Algorithm terminated.", false);
        }

    #ifdef QT_DEBUG
        // increases the chance that when the debugger stops the visualization shows the actual configuration of the system
        emit updateSystem(std::make_shared<System>(*system));
    #endif
    }
}

void Simulator::insertParticleAt(const int x, const int y){
  const Node node(x, y);
  system->insertParticleAt(node);
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

bool Simulator::getSystemValid()
{
    return system->getSystemState() == System::SystemState::Valid;
}

bool Simulator::getSystemDisconnected()
{
    return system->getSystemState() == System::SystemState::Disconnected;
}

bool Simulator::getSystemTerminated()
{
    return system->getSystemState() == System::SystemState::Terminated;
}

bool Simulator::getSystemDeadlocked()
{
    return system->getSystemState() == System::SystemState::Deadlocked;
}

int Simulator::getNumParticles() const
{
    return system->getNumParticles();
}

int Simulator::getNumNonStaticParticles() const
{
    return system->getNumNonStaticParticles();
}

int Simulator::getNumMovements() const
{
    return system->getNumMovements();
}

int Simulator::getNumRounds() const
{
    return system->getNumRounds();
}

void Simulator::setCheckConnectivity(bool b)
{
    System::checkConnectivity = b;
}

void Simulator::setRoundDuration(int ms)
{
    roundTimer->setInterval(ms);
    emit roundDurationChanged(ms);
}

void Simulator::saveScreenshotSlot(const QString filePath)
{
    emit saveScreenshotSignal(std::make_shared<System>(*system), filePath);
}
