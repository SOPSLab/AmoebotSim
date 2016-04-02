#include <set>
#include <QtGlobal>
#include <QDebug>
#include <math.h>

#include "alg/ising.h"

IsingParticle::IsingParticle(const Node head,
                             const int globalTailDir,
                             const int orientation,
                             AmoebotSystem& system,
                             Spin spin,
                             float beta)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      spin(spin),
      beta(beta),
      switchProb(-1)
{}

void IsingParticle::activate()
{
    int sum = sumNeighborSpins();
    int spinValue = (spin == Spin::Pos) ? -1 : 1; // sigma'(v) is the charge in the expected (switched) configuration
    switchProb = exp(beta * spinValue * sum) / (exp(beta * spinValue * sum) + exp(-1 * beta * spinValue * sum));
    qDebug() << switchProb;

    if(randFloat(0,1) < switchProb) {
        spin = (spin == Spin::Pos) ? Spin::Neg : Spin::Pos;
        qDebug() << "switched!";
    }

    return;
}

int IsingParticle::headMarkColor() const
{
    switch(spin) {
    case Spin::Neg:   return 0xff0000;
    case Spin::Pos:   return 0x0000ff;
    }

    return -1;
}

int IsingParticle::headMarkDir() const
{
    return -1; // no head is used in this algorithm
}

int IsingParticle::tailMarkColor() const
{
    return headMarkColor();
}

QString IsingParticle::inspectionText() const
{
    QString text;
    text += "head: (" + QString::number(head.x) + ", " + QString::number(head.y) + ")\n";
    text += "orientation: " + QString::number(orientation) + "\n";
    text += "globalTailDir: " + QString::number(globalTailDir) + "\n";
    text += "spin: ";
    text += [this](){
        switch(spin) {
        case Spin::Neg:   return "-1";
        case Spin::Pos:   return "+1";
        }
    }();
    text += "\n";
    return text;
}

IsingParticle& IsingParticle::neighborAtLabel(int label) const
{
    return AmoebotParticle::neighborAtLabel<IsingParticle>(label);
}

int IsingParticle::sumNeighborSpins()
{
    int sum = 0;

    for(int i = 0; i < 6; ++i) {
        if(hasNeighborAtLabel(i)) {
            sum = (neighborAtLabel(i).spin == Spin::Pos) ? sum + 1 : sum - 1;
        }
        else {
            sum -= 1; // empty holes should be treated as -1 spin particles
        }
    }

    return sum;
}

IsingSystem::IsingSystem(int numParticles, float beta)
{
    int posx, posy;
    for(int i = 1; i <= numParticles; ++i) {
        int layer = 1;
        int position = i - 1;
        while(position - (6 * layer) >= 0) {
            position -= 6 * layer;
            ++layer;
        }

        switch(position / layer) {
            case 0: {
                posx = layer;
                posy = (position % layer) - layer;
                if(position % layer == 0) {posx -= 1; posy += 1;} // addresses a corner case
                break;
            }
            case 1: {posx = layer - (position % layer); posy = position % layer; break;}
            case 2: {posx = -1 * (position % layer); posy = layer; break;}
            case 3: {posx = -1 * layer; posy = layer - (position % layer); break;}
            case 4: {posx = (position % layer) - layer; posy = -1 * (position % layer); break;}
            case 5: {posx = (position % layer); posy = -1 * layer; break;}
        }

        IsingParticle::Spin spin = (randBool()) ? IsingParticle::Spin::Neg : IsingParticle::Spin::Pos;

        insert(new IsingParticle(Node(posx, posy), -1, randDir(), *this, spin, beta));
    }
}

bool IsingSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    if(!isConnected(particles)) {
        return true;
    }
#endif

    IsingParticle::Spin spin;
    bool isFirstParticle = true;
    for(auto p : particles) {
        auto hp = dynamic_cast<IsingParticle*>(p);
        if(isFirstParticle) {
            spin = hp->spin;
            isFirstParticle = false;
        }
        else if(spin != hp->spin) {
            return false; // causes the system to terminate when all particles have the same spin
        }
    }

    return true;
}
