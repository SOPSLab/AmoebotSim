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
    // script commands
    void log(const QString msg, bool error = false);
    void runScript(const QString scriptFilePath);
    void writeToFile(const QString filePath, const QString text);

    // simulator flow controls
    void round();
    void runUntilTermination();

    int getNumParticles();
    int getNumMovements();
    int getNumRounds();
    int getLeaderElectionRounds();
    int getWeakBound();
    int getStrongBound();

    void setRoundDuration(int ms); 

    // visualization interface
    void focusOn(int x, int y);
    void setZoom(float zoom);

    // algorithms
    void adder(int numParticles = 10, int countValue = 250);
    void compression(int numParticles = 100, float lambda = 4.0);
    void hexagon(int numParticles = 200, float holeProb = 0.2);
    void ising(int numParticles = 200, float beta = 0.2);
    void linesort(int numParticles = 200, float holeProb = 0.2);
    void matrix(int numParticles = 10, int countValue = 250, int mode = 0);
    void rectangle(int numParticles = 200, float holeProb = 0.2);
    void sierpinski(int numParticles = 200, float holeProb = 0.2);
    void tokenDemo(int numParticles = 200, float holeProb = 0.2);
    void twositecbridge(int numParticles = 100, float lambda = 4.0, float alpha = 1.0);
    void twositeebridge(int numParticles = 200, float explambda = 2.0, float complambda = 4.0, float alpha = 1.0);

    // legacy algorithms
    void boundedObjCoating(const int numStaticParticles, const int numParticles, const float holeProb = 0.2);
    void compaction(const unsigned int numParticles = 100);
    void holeelimcompaction(const unsigned int numParticles = 100);
    void holeelimstandard(const unsigned int numParticles = 100);
    void infObjCoating(const int numParticles, const float holeProb = 0.2);
    void leaderelection(const unsigned int numParticles = 100);
    void leaderelectiondemo();
    void line(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void ring(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void square(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void triangle(const unsigned int numParticles = 100, const float holeProb = 0.0);
    void universalcoating(const int staticParticlesRadius = 5, const int numParticles = 50, const float holeProb = 0.2);

    // universal coating competitive analysis
    // int getUniversalCoatingWeakLowerBound();
    // int getUniversalCoatingStrongLowerBound();

private:
    ScriptEngine& engine;
    Simulator& sim;
    VisItem* vis;
};

#endif // SCRIPTINTERFACE_H
