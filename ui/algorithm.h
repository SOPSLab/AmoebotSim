/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#ifndef AMOEBOTSIM_UI_ALGORITHM_H_
#define AMOEBOTSIM_UI_ALGORITHM_H_

#include <memory>
#include <utility>
#include <vector>

#include <QObject>
#include <QString>
#include <QStringList>

#include "core/system.h"

class Algorithm : public QObject {
  Q_OBJECT

 public:
  // Constructs an algorithm with a given name (e.g., "Infinite Object Coating")
  // and a given signature (e.g., "infobjcoating").
  Algorithm(QString name, QString signature);

  // Typical getters and setters for the algorithm name and signature.
  QString getName() const;
  void setName(QString name);
  QString getSignature() const;
  void setSignature(QString signature);

  // Returns a list of this algorithm's parameter names or default values.
  QStringList getParameterNames() const;
  QStringList getParameterDefaults() const;

  // Adds a parameter to the algorithm of the given name and default value.
  void addParameter(QString parameter, QString defaultValue);

 signals:
  void log(const QString msg, bool error = false);
  void setSystem(std::shared_ptr<System> system);

 private:
  QString _name;
  QString _signature;
  std::vector<std::pair<QString, QString>> _parameters;
};

/* Algorithm classes for handling the instantiation of specific algorithms */

// Demo: Disco, a first tutorial.
class DiscoDemoAlg : public Algorithm {
  Q_OBJECT

 public:
  DiscoDemoAlg();

 public slots:
  void instantiate(const int numParticles = 30, const int counterMax = 5);
};

// Demo: Metrics.
class MetricsDemoAlg : public Algorithm {
  Q_OBJECT

 public:
  MetricsDemoAlg();

 public slots:
  void instantiate(const int numParticles = 30, const int counterMax = 5);
};

// Demo: Ballroom, a tutorial in coordination.
class BallroomDemoAlg : public Algorithm {
  Q_OBJECT

 public:
  BallroomDemoAlg();

 public slots:
  void instantiate(const int numParticles = 30);
};

// Demo: Token Passing.
class TokenDemoAlg : public Algorithm {
  Q_OBJECT

 public:
  TokenDemoAlg();

 public slots:
  void instantiate(const int numParticles = 48, const int lifetime = 100);
};

class DynamicDemoAlg : public Algorithm {
  Q_OBJECT

 public:
  DynamicDemoAlg();

 public slots:
  void instantiate(const unsigned int numParticles = 10,
                   const double growProb = 0.02, const double dieProb = 0.01);
};

// Aggregation.
class AggregationAlg : public Algorithm {
  Q_OBJECT

public:
  AggregationAlg();

public slots:
  void instantiate(const int numParticles = 2, const QString mode = "d",
                   const double noiseAmt = 3.0);
};

// Compression.
class CompressionAlg : public Algorithm {
  Q_OBJECT

 public:
  CompressionAlg();

 public slots:
  void instantiate(const int numParticles = 100, const double lambda = 4.0);
};

// Energy Distribution Framework + Hexagon Formation (canonical).
class EDFHexagonFormationAlg : public Algorithm {
  Q_OBJECT

 public:
  EDFHexagonFormationAlg();

 public slots:
  void instantiate(const int numParticles = 200, const int numEnergySources = 1,
                   const double holeProb = 0.2, const int capacity = 10,
                   const int transferRate = 1, const int demand = 5);
};

// Energy Distribution Framework + Leader Election by Erosion.
class EDFLeaderElectionByErosionAlg : public Algorithm {
  Q_OBJECT

 public:
  EDFLeaderElectionByErosionAlg();

 public slots:
  void instantiate(const int numParticles = 91, const int numEnergySources = 1,
                   const int capacity = 10, const int transferRate = 1,
                   const int demand = 5);
};

// Energy Distribution + Hexagon Formation.
class EnergyShapeAlg : public Algorithm {
  Q_OBJECT

 public:
  EnergyShapeAlg();

 public slots:
  void instantiate(const int numParticles = 200, const int numEnergyRoots = 1,
                   const double holeProb = 0.2, const double capacity = 10,
                   const double demand = 5, const double transferRate = 1);
};

// Energy Distribution/Sharing.
class EnergySharingAlg : public Algorithm {
  Q_OBJECT

 public:
  EnergySharingAlg();

 public slots:
  void instantiate(int numParticles = 91, const int numEnergyRoots = 1,
                   const int usage = 0, const double capacity = 10,
                   const double demand = 5, const double transferRate = 1);
};

// Hexagon Formation (canonical).
class HexagonFormationAlg : public Algorithm {
  Q_OBJECT

 public:
  HexagonFormationAlg();

 public slots:
  void instantiate(const int numParticles = 200, const double holeProb = 0.2);
};

// Infinite Object Coating.
class InfObjCoatingAlg : public Algorithm {
  Q_OBJECT

 public:
  InfObjCoatingAlg();

 public slots:
  void instantiate(const int numParticles = 100, const double holeProb = 0.2);
};

// Leader Election.
class LeaderElectionAlg : public Algorithm {
  Q_OBJECT

 public:
  LeaderElectionAlg();

 public slots:
  void instantiate(const int numParticles = 100, const double holeProb = 0.2);
};


// Leader Election by Erosion.
class LeaderElectionByErosionAlg : public Algorithm {
  Q_OBJECT

 public:
  LeaderElectionByErosionAlg();

 public slots:
  void instantiate(const int numParticles = 91);
};

// Basic Shape Formation.
class ShapeFormationAlg : public Algorithm {
  Q_OBJECT

 public:
  ShapeFormationAlg();

 public slots:
  void instantiate(const int numParticles = 200, const double holeProb = 0.2,
                   const QString mode = "h");
};

class AlgorithmList {
 public:
  // Constructs a list of algorithms recognized by the simulator (for now, this
  // mirrors ScriptInterface).
  AlgorithmList();

  // Destructs the Algorithms that are contained in this list.
  virtual ~AlgorithmList();

  // Returns a list of all the algorithms in this list.
  std::vector<Algorithm*> getAlgs();

  // Returns the algorithm object of the given algorithm.
  Algorithm* getAlg(QString algName) const;

  // Returns a list of all the algorithm's names in this list.
  QStringList getAlgNames() const;

  // Returns the algorithm signature of the given algorithm.
  QString getAlgSignature(QString algName) const;

  // Returns a list of all parameter names or default values of the given
  // algorithm.
  QStringList getParameterNames(QString algName) const;
  QStringList getParameterDefaults(QString algName) const;

 private:
  std::vector<Algorithm*> _algorithms;
};

#endif  // AMOEBOTSIM_UI_ALGORITHM_H_
