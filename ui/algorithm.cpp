/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "ui/algorithm.h"

#include "alg/demo/ballroomdemo.h"
#include "alg/demo/discodemo.h"
#include "alg/demo/metricsdemo.h"
#include "alg/demo/tokendemo.h"
#include "alg/compression.h"
#include "alg/energyshape.h"
#include "alg/energysharing.h"
#include "alg/infobjcoating.h"
#include "alg/leaderelection.h"
#include "alg/shapeformation.h"

Algorithm::Algorithm(QString name, QString signature)
    : _name(name),
      _signature(signature) {}

QString Algorithm::getName() const {
  return _name;
}

void Algorithm::setName(QString name) {
  _name = name;
}

QString Algorithm::getSignature() const {
  return _signature;
}

void Algorithm::setSignature(QString signature) {
  _signature = signature;
}

QStringList Algorithm::getParameterNames() const {
  QStringList names;
  for (auto parameter : _parameters) {
    names.append(parameter.first);
  }

  return names;
}

QStringList Algorithm::getParameterDefaults() const {
  QStringList defaults;
  for (auto parameter : _parameters) {
    defaults.append(parameter.second);
  }

  return defaults;
}

void Algorithm::addParameter(QString parameter, QString defaultValue) {
  _parameters.push_back(std::make_pair(parameter, defaultValue));
}

DiscoDemoAlg::DiscoDemoAlg() : Algorithm("Demo: Disco", "discodemo") {
  addParameter("# Particles", "30");
  addParameter("Counter Max", "5");
};

void DiscoDemoAlg::instantiate(const int numParticles, const int counterMax) {
  if (numParticles <= 0) {
    emit log("# particles must be > 0", true);
  } else if (counterMax <= 0) {
    emit log("counterMax must be > 0", true);
  } else {
    emit setSystem(std::make_shared<DiscoDemoSystem>(numParticles));
  }
}

MetricsDemoAlg::MetricsDemoAlg() : Algorithm("Demo: Metrics", "metricsdemo") {
  addParameter("# Particles", "30");
  addParameter("Counter Max", "5");
};

void MetricsDemoAlg::instantiate(const int numParticles, const int counterMax) {
  if (numParticles <= 0) {
    emit log("# particles must be > 0", true);
  } else if (counterMax <= 0) {
    emit log("counterMax must be > 0", true);
  } else {
    emit setSystem(std::make_shared<MetricsDemoSystem>(numParticles));
  }
}

BallroomDemoAlg::BallroomDemoAlg() : Algorithm("Demo: Ballroom", "ballroomdemo") {
  addParameter("# Particles", "30");
}

void BallroomDemoAlg::instantiate(const int numParticles) {
  emit setSystem(std::make_shared<BallroomDemoSystem>(numParticles));
}

TokenDemoAlg::TokenDemoAlg() : Algorithm("Demo: Token Passing", "tokendemo") {
  addParameter("# Particles", "48");
  addParameter("Token Lifetime", "100");
}

void TokenDemoAlg::instantiate(const int numParticles, const int lifetime) {
  if (numParticles <= 6) {
    emit log("# particles must be > 6", true);
  } else if (lifetime <= 0) {
    emit log("token lifetime must be > 0", true);
  } else {
    emit setSystem(std::make_shared<TokenDemoSystem>(numParticles, lifetime));
  }
}

CompressionAlg::CompressionAlg() : Algorithm("Compression", "compression") {
  addParameter("# Particles", "100");
  addParameter("Lambda", "4.0");
}

void CompressionAlg::instantiate(const int numParticles, const double lambda) {
  if (numParticles <= 0) {
    emit log("# particles must be > 0", true);
  } else {
    emit setSystem(std::make_shared<CompressionSystem>(numParticles, lambda));
  }
}

EnergyShapeAlg::EnergyShapeAlg()
    : Algorithm("Energy + Hexagon Formation", "energyshape") {
  addParameter("# Particles", "200");
  addParameter("# Energy Roots", "1");
  addParameter("Hole Prob.", "0.2");
  addParameter("Capacity", "10.0");
  addParameter("Demand", "1.0");
  addParameter("Harvest Rate", "1.0");
  addParameter("Battery Fraction", "0.75");
}

void EnergyShapeAlg::instantiate(const int numParticles,
                                 const int numEnergyRoots,
                                 const double holeProb,
                                 const double capacity, const double demand,
                                 const double harvestRate,
                                 const double batteryFrac) {
  if (numParticles <= 0) {
    emit log("# particles must be > 0", true);
  } else if (numEnergyRoots <= 0 || numEnergyRoots > numParticles) {
    emit log("# energy roots must be in (0, #particles]", true);
  } else if (holeProb < 0 || holeProb > 1) {
    emit log("holeProb in [0,1] required", true);
  } else if (capacity <= 0) {
    emit log("capacity must be > 0", true);
  } else if (demand < 0 || demand > capacity) {
    emit log("demand must be in [0, capacity]", true);
  } else if (harvestRate <= 0 || harvestRate > 0.5 * capacity) {
    emit log("harvestRate must be in (0, capacity/2]", true);
  } else if (batteryFrac <= 0 || batteryFrac >= 1) {
    emit log("batteryFrac must be in (0, 1)", true);
  } else {
    emit setSystem(std::make_shared<EnergyShapeSystem>(
                     numParticles, numEnergyRoots, holeProb, capacity, demand,
                     harvestRate, batteryFrac));
  }
}

EnergySharingAlg::EnergySharingAlg()
    : Algorithm("Energy Sharing", "energysharing") {
  addParameter("# Particles", "91");
  addParameter("Energy Usage", "0");
  addParameter("Capacity", "10.0");
  addParameter("Harvest Rate", "1.0");
  addParameter("Battery Fraction", "0.75");
}

void EnergySharingAlg::instantiate(int numParticles, const int usage,
                                   const double capacity,
                                   const double harvestRate,
                                   const double batteryFrac) {
  if (numParticles <= 0) {
    emit log("# particles must be > 0", true);
  } else if (usage < 0 || usage > 1) {
    emit log("usage mode must be 0 or 1");
  } else if (capacity <= 0) {
    emit log("capacity must be > 0", true);
  } else if (harvestRate <= 0 || harvestRate > 0.5 * capacity) {
    emit log("harvestRate must be in (0, capacity/2]", true);
  } else if (batteryFrac <= 0 || batteryFrac >= 1) {
    emit log("batteryFrac must be in (0, 1)", true);
  } else {
    emit setSystem(std::make_shared<EnergySharingSystem>(
                     numParticles, usage, capacity, harvestRate, batteryFrac));
  }
}

InfObjCoatingAlg::InfObjCoatingAlg() :
  Algorithm("Infinite Object Coating", "infobjcoating") {
  addParameter("# Particles", "100");
  addParameter("Hole Prob.", "0.2");
}

void InfObjCoatingAlg::instantiate(const int numParticles,
                                   const double holeProb) {
  if (numParticles <= 0) {
    emit log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    emit log("holeProb in [0,1] required", true);
  } else {
    emit setSystem(std::make_shared<InfObjCoatingSystem>(numParticles,
                                                         holeProb));
  }
}

LeaderElectionAlg::LeaderElectionAlg() :
  Algorithm("Leader Election", "leaderelection") {
  addParameter("# Particles", "100");
  addParameter("Hole Prob.", "0.2");
}

void LeaderElectionAlg::instantiate(const int numParticles,
                                    const double holeProb) {
  if (numParticles <= 0) {
    emit log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    emit log("holeProb in [0,1] required", true);
  } else {
    emit setSystem(std::make_shared<LeaderElectionSystem>(numParticles,
                                                          holeProb));
  }
}

ShapeFormationAlg::ShapeFormationAlg() :
  Algorithm("Basic Shape Formation", "shapeformation") {
  addParameter("# Particles", "200");
  addParameter("Hole Prob.", "0.2");
  addParameter("Shape", "h");
}

void ShapeFormationAlg::instantiate(const int numParticles,
                                    const double holeProb, const QString mode) {
  std::set<QString> set = ShapeFormationSystem::getAcceptedModes();
  if (numParticles <= 0) {
    emit log("# particles must be > 0", true);
  } else if (holeProb < 0 || holeProb > 1) {
    emit log("holeProb in [0,1] required", true);
  } else if (set.find(mode) == set.end()) {
    QString accepted = "";
    for(std::set<QString>::iterator it = set.begin(); it != set.end(); ++it) {
      if (accepted != "") accepted = accepted + ", " + *it;
      else accepted = *it;
    }
    emit log("only accepted modes are: " + accepted, true);
  } else {
    emit setSystem(std::make_shared<ShapeFormationSystem>(numParticles,
                                                          holeProb, mode));
  }
}

AlgorithmList::AlgorithmList() {
  // Demo algorithms.
  _algorithms.push_back(new DiscoDemoAlg());  
  _algorithms.push_back(new MetricsDemoAlg());
  _algorithms.push_back(new BallroomDemoAlg());  
  _algorithms.push_back(new TokenDemoAlg());

  // General algorithms.
  _algorithms.push_back(new CompressionAlg());
  _algorithms.push_back(new EnergyShapeAlg());
  _algorithms.push_back(new EnergySharingAlg());
  _algorithms.push_back(new InfObjCoatingAlg());    
  _algorithms.push_back(new LeaderElectionAlg());
  _algorithms.push_back(new ShapeFormationAlg());
}

AlgorithmList::~AlgorithmList() {
  _algorithms.erase(_algorithms.begin(), _algorithms.end());
}

std::vector<Algorithm*> AlgorithmList::getAlgs() {
  return _algorithms;
}

Algorithm* AlgorithmList::getAlg(QString algName) const {
  Algorithm* algo = nullptr;

  for (auto alg : _algorithms) {
    if (alg->getName().compare(algName) == 0) {
      algo = alg;
      break;
    }
  }

  return algo;
}

QStringList AlgorithmList::getAlgNames() const {
  QStringList names;
  for (auto alg : _algorithms) {
    names.append(alg->getName());
  }
  names.sort();

  return names;
}

QString AlgorithmList::getAlgSignature(QString algName) const {
  QString signature;
  for (auto alg : _algorithms) {
    if (alg->getName().compare(algName) == 0) {
      signature = alg->getSignature();
      break;
    }
  }

  return signature;
}

QStringList AlgorithmList::getParameterNames(QString algName) const {
  QStringList names;
  for (auto alg : _algorithms) {
    if (alg->getName().compare(algName) == 0) {
      names = alg->getParameterNames();
      break;
    }
  }

  return names;
}

QStringList AlgorithmList::getParameterDefaults(QString algName) const {
  QStringList defaults;
  for (auto alg : _algorithms) {
    if (alg->getName().compare(algName) == 0) {
      defaults = alg->getParameterDefaults();
      break;
    }
  }

  return defaults;
}
