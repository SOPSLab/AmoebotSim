#ifndef SCRIPTINTERFACE_H
#define SCRIPTINTERFACE_H

#include <QFile>
#include <QObject>
#include <QString>
#include <QTextStream>

#include "alg/legacy/legacysystem.h"

#include "alg/legacy/boundedobjcoating.h"
#include "alg/legacy/compaction.h"
#include "alg/legacy/compaction.h"
#include "alg/legacy/examplealgorithm.h"
#include "alg/legacy/hexagon.h"
#include "alg/legacy/holeelimcompaction.h"
#include "alg/legacy/holeelimstandard.h"
#include "alg/legacy/infobjcoating.h"
#include "alg/legacy/leaderelection.h"
#include "alg/legacy/leaderelectiondemo.h"
#include "alg/legacy/line.h"
#include "alg/legacy/ring.h"
#include "alg/legacy/square.h"
#include "alg/legacy/triangle.h"
#include "alg/legacy/universalcoating.h"

#include "alg/test.h"

#include "sim/simulator.h"

#include "helper/universalcoatinghelper.h"

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

    int getNumParticles();
    int getNumMovements();
    int getNumRounds();

    void setRoundDuration(int ms);

    void exampleAlgorithm(const int numParticles);
    void infObjCoating(const int numParticles, const float holeProb = 0.2);
    void line(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void boundedObjCoating(const int numStaticParticles, const int numParticles, const float holeProb = 0.2);
    void hexagon(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void triangle(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void ring(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void square(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void compaction(const unsigned int numParticles = 100);
    void holeelimstandard(const unsigned int numParticles = 100);
    void holeelimcompaction(const unsigned int numParticles = 100);
    void leaderelection(const unsigned int numParticles = 100);
    void universalcoating(const int staticParticlesRadius = 5, const int numParticles = 50, const float holeProb = 0.2);
    void leaderelectiondemo();

//    int getUniversalCoatingWeakLowerBound();
//    int getUniversalCoatingStrongLowerBound();

private:
    Simulator& sim;
};

inline ScriptInterface::ScriptInterface(Simulator& _sim)
    : sim(_sim)
{
    sim.setSystem(std::make_shared<Test::TestSystem>());
//    sim.setSystem(Hexagon::Hexagon::instance(200, 0.3));
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

inline int ScriptInterface::getNumParticles()
{
    return sim.getNumParticles();
}

inline int ScriptInterface::getNumMovements()
{
    return sim.getNumMovements();
}

inline int ScriptInterface::getNumRounds()
{
    return sim.getNumRounds();
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

inline void ScriptInterface::boundedObjCoating(const int numStaticParticles, const int numParticles, const float holeProb)
{
    if(numParticles < 0) {
        sim.log("numParticles >= 0 required", true);
        return;
    }

    if(holeProb < 0.0f || holeProb > 1.0f) {
        sim.log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(BoundedObjCoating::BoundedObjCoating::instance(numStaticParticles, numParticles, holeProb));
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

inline void ScriptInterface::square(const unsigned int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        sim.log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Square::Square::instance(numParticles, holeProb));
}

inline void ScriptInterface::compaction(const unsigned int numParticles)
{
    sim.setSystem(Compaction::Compaction::instance(numParticles));
}

inline void ScriptInterface::holeelimstandard(const unsigned int numParticles)
{
    sim.setSystem(HoleElimStandard::HoleElimStandard::instance(numParticles));
}

inline void ScriptInterface::holeelimcompaction(const unsigned int numParticles)
{
    sim.setSystem(HoleElimCompaction::HoleElimCompaction::instance(numParticles));
}

inline void ScriptInterface::leaderelection(const unsigned int numParticles)
{
    sim.setSystem(LeaderElection::LeaderElection::instance(numParticles));
}

inline void ScriptInterface::leaderelectiondemo()
{
    sim.setSystem(LeaderElectionDemo::LeaderElectionDemo::instance());
}

inline void ScriptInterface::universalcoating(const  int staticParticlesRadius, const int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        sim.log("holeProb in [0.0, 1.0] required", true);
        return;
    }
    sim.setSystem(UniversalCoating::UniversalCoating::instance(staticParticlesRadius, numParticles, holeProb));
}

//inline int ScriptInterface::getUniversalCoatingWeakLowerBound()
//{
//    return UniversalCoating::getWeakLowerBound(*sim.getSystem());
//}

//inline int ScriptInterface::getUniversalCoatingStrongLowerBound()
//{
//    return UniversalCoating::getStrongLowerBound(*sim.getSystem());
//}

#endif // SCRIPTINTERFACE_H
