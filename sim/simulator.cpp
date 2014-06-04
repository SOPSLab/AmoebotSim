#include <QObject>

#include "alg/dummyalg.h"
#include "sim/node.h"
#include "sim/particle.h"
#include "sim/simulator.h"

Simulator::Simulator()
    : roundTimer(nullptr)
{
    for(int x = 0; x < 6; x++) {
        system.insert(Particle(new DummyAlg(), 0, Node(x, 0), -1));
    }
}

void Simulator::init()
{
    if(roundTimer == nullptr) {
        roundTimer = new QTimer(this);
        roundTimer->setInterval(100);
        connect(roundTimer, &QTimer::timeout, this, &Simulator::round);
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
