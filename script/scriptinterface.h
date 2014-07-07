#ifndef SCRIPTINTERFACE_H
#define SCRIPTINTERFACE_H

#include <QObject>

#include "alg/examplealgorithm.h"
#include "alg/infobjcoating.h"
#include "alg/hexagon.h"
#include "alg/triangle.h"
#include "alg/ring.h"
#include "sim/simulator.h"

/*
 * The methods of the following class are automatically available during runtime as command in the command-line.
 * */
class ScriptInterface : public QObject
{
    Q_OBJECT
public:
    explicit ScriptInterface(Simulator& _sim);
    
public slots:
    void round();

    void setRoundDuration(int ms);
    int getNumMovements();

    void exampleAlgorithm(const int numParticles);
    void infObjCoating(const int numParticles, const float holeProb = 0.2);
    void hexagon(const int numParticles = 100, const float holeProb = 0.0);
    void triangle(const int numParticles = 100, const float holeProb = 0.0);
    void ring(const int numParticles = 100, const float holeProb = 0.0);

private:
    Simulator& sim;
};

inline ScriptInterface::ScriptInterface(Simulator& _sim)
    : sim(_sim)
{
    sim.setSystem(InfObjCoating::InfObjCoating::instance(100, 0.2));
}

inline void ScriptInterface::round()
{
    sim.round();
}

inline void ScriptInterface::setRoundDuration(int ms)
{
    sim.setRoundDuration(ms);
}

inline int ScriptInterface::getNumMovements()
{
    return sim.getNumMovements();
}

inline void ScriptInterface::exampleAlgorithm(const int numParticles)
{
    if(numParticles < 0) {
        sim.log("numParticles >= 0 required", true);
        return;
    }

    sim.setSystem(ExampleAlgorithm::ExampleAlgorithm::instance(numParticles));
}

inline void ScriptInterface::infObjCoating(const int numParticles, const float holeProb)
{
    if(numParticles < 0) {
        sim.log("numParticles >= 0 required", true);
        return;
    }

    if(holeProb < 0.0f || holeProb > 1.0f) {
        sim.log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(InfObjCoating::InfObjCoating::instance(numParticles, holeProb));
}

inline void ScriptInterface::hexagon(const int numParticles, const float holeProb)
{
    if(numParticles < 0) {
        sim.log("numParticles >= 0 required", true);
        return;
    }

    if(holeProb < 0.0f || holeProb > 1.0f) {
        sim.log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Hexagon::Hexagon::instance(numParticles, holeProb));
}

inline void ScriptInterface::triangle(const int numParticles, const float holeProb)
{
    if(numParticles < 0) {
        sim.log("numParticles >= 0 required", true);
        return;
    }

    if(holeProb < 0.0f || holeProb > 1.0f) {
        sim.log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Triangle::Triangle::instance(numParticles, holeProb));
}
inline void ScriptInterface::ring(const int numParticles, const float holeProb)
{
    if(numParticles < 0) {
        sim.log("numParticles >= 0 required", true);
        return;
    }

    if(holeProb < 0.0f || holeProb > 1.0f) {
        sim.log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Ring::Ring::instance(numParticles, holeProb));
}

#endif // SCRIPTINTERFACE_H
