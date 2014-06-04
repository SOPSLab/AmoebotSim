#include "simulator.h"

#include "alg/dummyalg.h"
#include "sim/node.h"
#include "sim/particle.h"

Simulator::Simulator(QObject* parent) :
    QObject(parent)
{
    for(int x = 0; x < 6; x++) {
        system.insert(Particle(new DummyAlg(), 0, Node(x, 0), -1));
    }

    roundTimer.setInterval(100);
    connect(&roundTimer, &QTimer::timeout, this, &Simulator::round);
}

void Simulator::round()
{
    system.round();
    emit updateSystem(new System(system));
}

void Simulator::start()
{
    roundTimer.start();
}

void Simulator::stop()
{
    roundTimer.stop();
}
