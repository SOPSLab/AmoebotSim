#include "simulator.h"

Simulator::Simulator(QObject *parent) :
    QObject(parent)
{
    Particle p;
    p.pos.x = 0;
    p.pos.y = 0;
    system.particles.push_back(p);

    roundTimer.setInterval(1000);
    connect(&roundTimer, &QTimer::timeout, this, &Simulator::round);
}

void Simulator::round()
{
    system.particles[0].pos.x = system.particles[0].pos.x + 1;
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
