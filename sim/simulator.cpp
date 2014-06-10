#include <QScriptValue>

#include "script/scriptinterface.h"
#include "sim/simulator.h"

Simulator::Simulator()
    : roundTimer(nullptr)
{
    engine.setProcessEventsInterval(33);
    engine.setGlobalObject(engine.newQObject(new ScriptInterface(*this), QScriptEngine::ScriptOwnership));
}

void Simulator::init()
{
    if(roundTimer == nullptr) {
        roundTimer = new QTimer(this);
        roundTimer->setInterval(100);
        connect(roundTimer, &QTimer::timeout, this, &Simulator::round);
        emit updateSystem(new System(system));
    }
}

void Simulator::round()
{
    system.round();
    emit updateSystem(new System(system));
}

void Simulator::start()
{
    roundTimer->start();
}

void Simulator::stop()
{
    roundTimer->stop();
}

void Simulator::executeCommand(const QString cmd)
{
    QScriptValue result = engine.evaluate(cmd);
    if(!result.isUndefined()) {
        emit log(result.toString(), result.isError());
    }
}
