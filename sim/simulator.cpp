#include "simulator.h"

#include "alg/dummyalg.h"

Simulator::Simulator(QObject* parent) :
    QObject(parent)
{
    for(int x = 0; x < 6; x++) {
        Particle p(0, Vec(x, 0), -1);
        p.setAlgorithm(new DummyAlg());
        system.insert(p);
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
