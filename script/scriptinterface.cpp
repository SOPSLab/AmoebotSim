#include <QFile>
#include <QTextStream>

#include "alg/legacy/legacysystem.h"
#include "alg/legacy/boundedobjcoating.h"
#include "alg/legacy/compaction.h"
#include "alg/legacy/compaction.h"
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

#include "alg/adder.h"
#include "alg/compression.h"
#include "alg/hexagon.h"
#include "alg/sierpinski.h"
#include "alg/rectangle.h"
#include "alg/ising.h"
#include "alg/tokendemo.h"
//#include "helper/universalcoatinghelper.h"

#include "script/scriptinterface.h"

#include "sim/node.h"

ScriptInterface::ScriptInterface(ScriptEngine &engine, Simulator& sim, VisItem *vis)
    : engine(engine), sim(sim), vis(vis)
{
    hexagon();
}

void ScriptInterface::log(const QString msg, bool error)
{
    emit engine.log(msg, error);
}

void ScriptInterface::runScript(const QString scriptFilePath)
{
    engine.runScript(scriptFilePath);
}

void ScriptInterface::writeToFile(const QString filePath, const QString text)
{
    QFile file(filePath);

    if(!file.open(QFile::WriteOnly | QFile::Append)) {
        log("could not write to file", true);
        return;
    }

    QTextStream stream(&file);
    stream << text;

    file.close();
}

void ScriptInterface::round()
{
    sim.round();
}

void ScriptInterface::runUntilTermination()
{
    sim.runUntilTermination();
}

int ScriptInterface::getNumParticles()
{
    return sim.numParticles();
}

int ScriptInterface::getNumMovements()
{
    return sim.numMovements();
}

int ScriptInterface::getNumRounds()
{
    return sim.numRounds();
}
int ScriptInterface::getLeaderElectionRounds()
{
    return sim.leaderElectionRounds();
}

int ScriptInterface::getWeakBound()
{
 return sim.weakBounds();
}
int ScriptInterface::getStrongBound()
{
 return sim.strongBounds();
}



void ScriptInterface::setRoundDuration(int ms)
{
    sim.setRoundDuration(ms);
}

void ScriptInterface::focusOn(int x, int y)
{
    if(vis != nullptr) {
        vis->focusOn(Node(x, y));
    }
}

void ScriptInterface::setZoom(float zoom)
{
    if(vis != nullptr) {
        vis->setZoom(zoom);
    }
}

void ScriptInterface::adder(int numParticles, int countValue)
{
    sim.setSystem(std::make_shared<AdderSystem>(numParticles, countValue));
}

void ScriptInterface::compression(int numParticles, float lambda)
{
    sim.setSystem(std::make_shared<CompressionSystem>(numParticles, lambda));
}

void ScriptInterface::hexagon(int numParticles, float holeProb)
{
    sim.setSystem(std::make_shared<HexagonSystem>(numParticles, holeProb));
}
void ScriptInterface::sierpinski(int numParticles, float holeProb)
{
    sim.setSystem(std::make_shared<SierpinskiSystem>(numParticles, holeProb));
}
void ScriptInterface::rectangle(int numParticles, float holeProb)
{
    sim.setSystem(std::make_shared<RectangleSystem>(numParticles, holeProb));
}
void ScriptInterface::ising(int numParticles, float beta)
{
    sim.setSystem(std::make_shared<IsingSystem>(numParticles, beta));
}

void ScriptInterface::tokenDemo(int numParticles, float holeProb)
{
    sim.setSystem(std::make_shared<TokenDemoSystem>(numParticles, holeProb));
}

//int ScriptInterface::getUniversalCoatingWeakLowerBound()
//{
//    return UniversalCoating::getWeakLowerBound(*sim.getSystem());
//}

//int ScriptInterface::getUniversalCoatingStrongLowerBound()
//{
//    return UniversalCoating::getStrongLowerBound(*sim.getSystem());
//}

void ScriptInterface::infObjCoating(const int numParticles, const float holeProb)
{
    if(numParticles < 0) {
        log("numParticles >= 0 required", true);
        return;
    }

    if(holeProb < 0.0f || holeProb > 1.0f) {
        log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(InfObjCoating::InfObjCoating::instance(numParticles, holeProb));
}

void ScriptInterface::boundedObjCoating(const int numStaticParticles, const int numParticles, const float holeProb)
{
    if(numParticles < 0) {
        log("numParticles >= 0 required", true);
        return;
    }

    if(holeProb < 0.0f || holeProb > 1.0f) {
        log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(BoundedObjCoating::BoundedObjCoating::instance(numStaticParticles, numParticles, holeProb));
}

void ScriptInterface::line(const unsigned int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Line::Line::instance(numParticles, holeProb));
}

void ScriptInterface::triangle(const unsigned int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Triangle::Triangle::instance(numParticles, holeProb));
}

void ScriptInterface::ring(const unsigned int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Ring::Ring::instance(numParticles, holeProb));
}

void ScriptInterface::square(const unsigned int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Square::Square::instance(numParticles, holeProb));
}

void ScriptInterface::compaction(const unsigned int numParticles)
{
    sim.setSystem(Compaction::Compaction::instance(numParticles));
}

void ScriptInterface::holeelimstandard(const unsigned int numParticles)
{
    sim.setSystem(HoleElimStandard::HoleElimStandard::instance(numParticles));
}

void ScriptInterface::holeelimcompaction(const unsigned int numParticles)
{
    sim.setSystem(HoleElimCompaction::HoleElimCompaction::instance(numParticles));
}

void ScriptInterface::leaderelection(const unsigned int numParticles)
{
    sim.setSystem(LeaderElection::LeaderElection::instance(numParticles));
}

void ScriptInterface::leaderelectiondemo()
{
    sim.setSystem(LeaderElectionDemo::LeaderElectionDemo::instance());
}

void ScriptInterface::universalcoating(const  int staticParticlesRadius, const int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        log("holeProb in [0.0, 1.0] required", true);
        return;
    }
    sim.setSystem(UniversalCoating::UniversalCoating::instance(staticParticlesRadius, numParticles, holeProb));
}
