#include <QScriptValue>

#include "script/scriptinterface.h"
#include "sim/simulator.h"

Simulator::Simulator()
    : roundTimer(nullptr),
      system(nullptr)
{
    engine.setProcessEventsInterval(33);
    engine.setGlobalObject(engine.newQObject(new ScriptInterface(*this), QScriptEngine::ScriptOwnership));
}

Simulator::~Simulator()
{
    delete system;
}

void Simulator::setSystem(System* _system)
{
    if(roundTimer != nullptr) {
        roundTimer->stop();
        emit stopped();
    }
    delete system;
    system = _system;
    emit updateSystem(new System(*system));
}

void Simulator::init()
{
    if(roundTimer == nullptr) {
        roundTimer = new QTimer(this);
        roundTimer->setInterval(100);
        connect(roundTimer, &QTimer::timeout, this, &Simulator::round);
        emit updateSystem(new System(*system));
    }
}

void Simulator::round()
{
    system->round();
    auto systemState = system->getSystemState();
    if(systemState == System::SystemState::Collision) {
        log("Collision detected. Simulation aborted.", true);
        roundTimer->stop();
        emit stopped();
    } else if(systemState == System::SystemState::Deadlock) {
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

    emit updateSystem(new System(*system));
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

void Simulator::executeCommand(const QString cmd)
{
    QScriptValue result = engine.evaluate(cmd);
    if(!result.isUndefined()) {
        emit log(result.toString(), result.isError());
    }
}
