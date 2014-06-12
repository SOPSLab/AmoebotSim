#include <QScriptValue>
#include <QTimer>

#include "script/scriptinterface.h"
#include "sim/simulator.h"

Simulator::Simulator()
    : roundTimer(nullptr),
      system(nullptr)
{
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
}

void Simulator::init()
{
    if(roundTimer == nullptr) {
        roundTimer = new QTimer(this);
        roundTimer->setInterval(100);
        connect(roundTimer, &QTimer::timeout, this, &Simulator::round);

        updateTimer = new QTimer(this);
        updateTimer->setInterval(33);
        connect(updateTimer, &QTimer::timeout, [&](){emit updateSystem(new System(*system));});
        updateTimer->start();
    }
}

void Simulator::round()
{
    system->round();
    auto systemState = system->getSystemState();
    if(systemState == System::SystemState::Deadlock) {
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

void Simulator::executeCommand(const QString cmd)
{
    QScriptValue result = engine.evaluate(cmd);
    if(!result.isUndefined()) {
        emit log(result.toString(), result.isError());
    }
}
