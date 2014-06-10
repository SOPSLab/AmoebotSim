#ifndef SCRIPTINTERFACE_H
#define SCRIPTINTERFACE_H

#include <QObject>

#include "alg/examplealgorithm.h"
#include "sim/simulator.h"

class ScriptInterface : public QObject
{
    Q_OBJECT
public:
    explicit ScriptInterface(Simulator& _sim);
    
public slots:
    void round();

private:
    Simulator& sim;
};

inline ScriptInterface::ScriptInterface(Simulator& _sim)
    : sim(_sim)
{
    _sim.system = ExampleAlgorithm::instance();
}

inline void ScriptInterface::round()
{
    sim.round();
}

#endif // SCRIPTINTERFACE_H
