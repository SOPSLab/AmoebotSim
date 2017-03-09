#include <QFile>
#include <QTextStream>
#include <QTime>
#include <math.h>

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

#include "alg/2sitecbridge.h"
#include "alg/adder.h"
#include "alg/compression.h"
#include "alg/hexagon.h"
#include "alg/ising.h"
#include "alg/linesort.h"
#include "alg/matrix.h"
#include "alg/matrix2.h"
#include "alg/edgedetect.h"
#include "alg/rectangle.h"
#include "alg/sierpinski.h"
#include "alg/tokendemo.h"
#include "alg/twositeebridge.h"

//#include "helper/universalcoatinghelper.h"

#include "script/scriptinterface.h"

#include "sim/node.h"

// constructor

ScriptInterface::ScriptInterface(ScriptEngine &engine, Simulator& sim, VisItem *vis)
    : engine(engine), sim(sim), vis(vis)
{
    hexagon();
}


// script commands
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

// simulation flow controls
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

// visualization interface
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
void ScriptInterface::saveScreenshot(QString filePath)
{
    if(filePath == "") {
        filePath = QString("amoebotsim_") + QString::number(QTime::currentTime().msecsSinceStartOfDay()) + QString(".png");
    }
    sim.saveScreenshotSetup(filePath);
}
void ScriptInterface::filmSimulation(QString filePath, const int iterLimit)
{
    const int filenameLength = 10;
    int i = 0;

    while(!sim.getSystem()->hasTerminated() && i < iterLimit && i < 10 * filenameLength) {
        emit vis->beforeRendering(); // updates GUI #rounds and #movements labels
        saveScreenshot(filePath + pad(i,filenameLength) + QString(".png"));
        round();
        ++i;
    }
}
QString ScriptInterface::pad(const int number, const int length)
{
    QString str = "" + QString::number(number);
    while(str.length() < length) {
        str = QString("0") + str;
    }
    return str;
}

// algorithms
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
void ScriptInterface::ising(int numParticles, float beta)
{
    sim.setSystem(std::make_shared<IsingSystem>(numParticles, beta));
}
void ScriptInterface::linesort(int numParticles, float holeProb)
{
    sim.setSystem(std::make_shared<LineSortSystem>(numParticles, holeProb));
}
void ScriptInterface::matrix(int numParticles, int countValue, int whichStream, int mode)
{
    if(mode == 0) {
        sim.setSystem(std::make_shared<Matrix2System>(numParticles, countValue,whichStream));
    }
    else if(mode == 1) {
        sim.setSystem(std::make_shared<MatrixSystem>(numParticles, countValue));
    }
}
void ScriptInterface::edgedetect(int numParticles, int countValue, int mode)
{
    sim.setSystem(std::make_shared<EdgeDetectSystem>(numParticles, countValue));
}
void ScriptInterface::rectangle(int numParticles, float holeProb)
{
    sim.setSystem(std::make_shared<RectangleSystem>(numParticles, holeProb));
}
void ScriptInterface::sierpinski(int numParticles, float holeProb)
{
    sim.setSystem(std::make_shared<SierpinskiSystem>(numParticles, holeProb));
}
void ScriptInterface::tokenDemo(int numParticles, float holeProb)
{
    sim.setSystem(std::make_shared<TokenDemoSystem>(numParticles, holeProb));
}
void ScriptInterface::twositecbridge(int numParticles, float lambda, float alpha)
{
    if(numParticles < 5) {
        log("# particles >= 5 required", true);
        return;
    }
    else if(lambda <= 1) {
        log("lambda > 1 required", true);
        return;
    }
    else if(alpha < 1) {
        log("alpha >= 1 required", true);
        return;
    }

    sim.setSystem(std::make_shared<TwoSiteCBridgeSystem>(numParticles, lambda, alpha));
}
void ScriptInterface::twositeebridge(int numParticles, float explambda, float complambda, float siteDistance)
{
    if(numParticles < 5) {
        log("# particles >= 5 required", true);
        return;
    }
    else if(explambda <= 0 || explambda >= 2.17) {
        log("expansion lambda must be > 0 and < 2.17", true);
        return;
    }
    else if(complambda <= 3.42) {
        log("compression lambda must be > 3.42", true);
        return;
    }
    else if(siteDistance <= 1 || siteDistance > sqrt(numParticles)) {
        log("site distance factor must be > 1 and < sqrt(#particles)", true);
        return;
    }

    sim.setSystem(std::make_shared<TwoSiteEBridgeSystem>(numParticles, explambda, complambda, siteDistance));
}


// legacy algorithms
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
void ScriptInterface::compaction(const unsigned int numParticles)
{
    sim.setSystem(Compaction::Compaction::instance(numParticles));
}
void ScriptInterface::holeelimcompaction(const unsigned int numParticles)
{
    sim.setSystem(HoleElimCompaction::HoleElimCompaction::instance(numParticles));
}
void ScriptInterface::holeelimstandard(const unsigned int numParticles)
{
    sim.setSystem(HoleElimStandard::HoleElimStandard::instance(numParticles));
}
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
void ScriptInterface::leaderelection(const unsigned int numParticles)
{
    sim.setSystem(LeaderElection::LeaderElection::instance(numParticles));
}
void ScriptInterface::leaderelectiondemo()
{
    sim.setSystem(LeaderElectionDemo::LeaderElectionDemo::instance());
}
void ScriptInterface::line(const unsigned int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Line::Line::instance(numParticles, holeProb));
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
void ScriptInterface::triangle(const unsigned int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        log("holeProb in [0.0, 1.0] required", true);
        return;
    }

    sim.setSystem(Triangle::Triangle::instance(numParticles, holeProb));
}
void ScriptInterface::universalcoating(const  int staticParticlesRadius, const int numParticles, const float holeProb)
{
    if(holeProb < 0.0f || holeProb > 1.0f) {
        log("holeProb in [0.0, 1.0] required", true);
        return;
    }
    sim.setSystem(UniversalCoating::UniversalCoating::instance(staticParticlesRadius, numParticles, holeProb));
}


// universal coating competitive analysis
/*int ScriptInterface::getUniversalCoatingWeakLowerBound()
{
    return UniversalCoating::getWeakLowerBound(*sim.getSystem());
}
int ScriptInterface::getUniversalCoatingStrongLowerBound()
{
    return UniversalCoating::getStrongLowerBound(*sim.getSystem());
}*/
