#ifndef SCRIPTINTERFACE_H
#define SCRIPTINTERFACE_H

#include <QFile>
#include <QObject>
#include <QString>
#include <QTextStream>

#include "alg/examplealgorithm.h"
#include "alg/infobjcoating.h"
#include "alg/boundedobjcoating.h"
#include "alg/line.h"
#include "alg/hexagon.h"
#include "alg/triangle.h"
#include "alg/ring.h"
#include "sim/simulator.h"
#include "alg/square.h"
#include "alg/compaction.h"
#include "alg/holeelimstandard.h"
#include "alg/holeelimcompaction.h"
#include "alg/leaderelection.h"
#include "alg/universalcoating.h"
#include "alg/leaderelectiondemo.h"
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
    void saveScreenshot(const QString filePath = "");

    bool isValid();
    bool isDisconnected();
    bool isTerminated();
    bool isDeadlocked();

    int getNumParticles();
    int getNumNonStaticParticles();
    int getNumMovements();
    int getNumRounds();

    void setCheckConnectivity(bool b);
    void setRoundDuration(int ms);
    void moveCameraTo(float worldX, float worldY);
    void setZoom(float factor);
    void setResolution(const int width, const int height);
    void setFullscreen();
    void setWindowed();

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
private:
    Simulator& sim;
};

inline ScriptInterface::ScriptInterface(Simulator& _sim)
    : sim(_sim)
{
    sim.setSystem(UniversalCoating::UniversalCoating::instance(2,120,0));
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

inline void ScriptInterface::saveScreenshot(const QString filePath)
{
    sim.saveScreenshotSlot(filePath);
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

inline int ScriptInterface::getNumNonStaticParticles()
{
    return sim.getNumNonStaticParticles();
}

inline int ScriptInterface::getNumMovements()
{
    return sim.getNumMovements();
}

inline int ScriptInterface::getNumRounds()
{
    return sim.getNumRounds();
}

inline void ScriptInterface::setCheckConnectivity(bool b)
{
    sim.setCheckConnectivity(b);
}

inline void ScriptInterface::setRoundDuration(int ms)
{
    sim.setRoundDuration(ms);
}

inline void ScriptInterface::moveCameraTo(float worldX, float worldY){
  emit sim.moveCameraTo(worldX, worldY);
}

inline void ScriptInterface::setZoom(float factor){
  emit sim.setZoom(factor);
}

inline void ScriptInterface::setFullscreen(){
  emit sim.setFullscreen();
}

inline void ScriptInterface::setWindowed(){
  emit sim.setWindowed();
}

inline void ScriptInterface::setResolution(const int width, const int height){
  emit sim.setResolution(width, height);
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

#endif // SCRIPTINTERFACE_H
