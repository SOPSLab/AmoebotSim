#ifndef SCRIPTINTERFACE_H
#define SCRIPTINTERFACE_H

#include <QFile>
#include <QObject>
#include <QString>
#include <QTextStream>

#include "alg/examplealgorithm.h"
#include "alg/infobjcoating.h"
#include "alg/line.h"
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

    void runScript(const QString scriptFilePath);
    void writeToFile(const QString filePath, const QString text);

    bool isValid();
    bool isDisconnected();
    bool isTerminated();
    bool isDeadlocked();

    int getNumParticles();
    int getNumMovements();

    void setRoundDuration(int ms);

    void exampleAlgorithm(const int numParticles);
    void infObjCoating(const int numParticles, const float holeProb = 0.2);
    void line(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void hexagon(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void triangle(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void ring(const unsigned int numParticles = 100, const float holeProb = 0.0);

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

inline void ScriptInterface::runScript(const QString scriptFilePath)
{
    QFile scriptFile(scriptFilePath);

    if(!scriptFile.open(QFile::ReadOnly)) {
        sim.log("could not open script file", true);
        return;
    }

    QTextStream stream(&scriptFile);
    QString script = stream.readAll();

    scriptFile.close();

    sim.runScript(script);
    sim.log("script finished", false);
}

inline void ScriptInterface::writeToFile(const QString filePath, const QString text)
{
    QFile file(filePath);

    if(!file.open(QFile::WriteOnly | QFile::Append)) {
        sim.log("could not write to file", true);
        return;
    }

    QTextStream stream(&file);
    stream << text;

    file.close();
}

inline bool ScriptInterface::isValid()
{
    return sim.getSystemValid();
}

inline bool ScriptInterface::isDisconnected()
{
    return sim.getSystemDisconnected();
}

inline bool ScriptInterface::isTerminated()
{
    return sim.getSystemTerminated();
}

inline bool ScriptInterface::isDeadlocked()
{
    return sim.getSystemDeadlocked();
}

inline int ScriptInterface::getNumParticles()
{
    return sim.getNumParticles();
}

inline int ScriptInterface::getNumMovements()
{
    return sim.getNumMovements();
}

inline void ScriptInterface::setRoundDuration(int ms)
{
    sim.setRoundDuration(ms);
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

inline void ScriptInterface::line(const unsigned int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        sim.log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Line::Line::instance(numParticles, holeProb));
}

inline void ScriptInterface::hexagon(const unsigned int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        sim.log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Hexagon::Hexagon::instance(numParticles, holeProb));
}

inline void ScriptInterface::triangle(const unsigned int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        sim.log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Triangle::Triangle::instance(numParticles, holeProb));
}
inline void ScriptInterface::ring(const unsigned int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        sim.log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Ring::Ring::instance(numParticles, holeProb));
}

#endif // SCRIPTINTERFACE_H
