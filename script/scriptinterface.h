#ifndef SCRIPTINTERFACE_H
#define SCRIPTINTERFACE_H

#include <QObject>
#include <QString>

#include "script/scriptengine.h"

#include "sim/simulator.h"

#include "ui/visitem.h"

/*
 * The public slots of the following class are automatically available during runtime as command in the command-line.
 * */
class ScriptInterface : public QObject
{
    Q_OBJECT
public:
    explicit ScriptInterface(ScriptEngine& engine, Simulator& sim, VisItem* vis);
    
public slots:
    // scripting stuff
    void log(const QString msg, bool error = false);
    void runScript(const QString scriptFilePath);
    void writeToFile(const QString filePath, const QString text);

    // interface to simulator
    void round();
    void runUntilTermination();

    int getNumParticles();
    int getNumMovements();
    int getNumRounds();
    int getLeaderElectionRounds();
    int getWeakBound();
    int getStrongBound();

    void setRoundDuration(int ms); 

    // interface to visualization
    void focusOn(int x, int y);
    void setZoom(float zoom);

    // algorithms
    void adder(int numParticles = 10, int countValue = 250);
    void compression(int numParticles = 100, float lambda = 4.0);
    void hexagon(int numParticles = 200, float holeProb = 0.2);
    void rectangle(int numParticles = 200, float holeProb = 0.2);
    void ising(int numParticles = 200, float beta = 0.2);
    void tokenDemo(int numParticles = 200, float holeProb = 0.2);

//    int getUniversalCoatingWeakLowerBound();
//    int getUniversalCoatingStrongLowerBound();

    // legacy algorithms
    void infObjCoating(const int numParticles, const float holeProb = 0.2);
    void line(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void boundedObjCoating(const int numStaticParticles, const int numParticles, const float holeProb = 0.2);
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
    ScriptEngine& engine;
    Simulator& sim;
    VisItem* vis;
};

#endif // SCRIPTINTERFACE_H
