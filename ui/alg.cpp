#include "ui/alg.h"

Algorithm::Algorithm(QString name, QString signature) :
  _name(name),
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
    names.append(parameter.second);
  }

  return names;
}

void Algorithm::addParameter(QString type, QString parameter) {
  _parameters.push_back(std::make_pair(type, parameter));
}

AlgorithmList::AlgorithmList() {
  // Adder. (TODO)
  _algorithms.push_back(Algorithm("Adder", "adder"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("int", "countValue");

  // Aggregation.
  _algorithms.push_back(Algorithm("Swarm Aggregation", "aggregation"));
  _algorithms.back().addParameter("int", "numParticles");

  // Compaction.
  _algorithms.push_back(Algorithm("Compaction", "compaction"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "holeProb");

  // Compression.
  _algorithms.push_back(Algorithm("Compression", "compression"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "lambda");

  // Convex Hull Formation.
  _algorithms.push_back(Algorithm("Convex Hull Formation", "convexhull"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("int", "numObjects");
  _algorithms.back().addParameter("double", "holeProb");

  // Edge Detection.
  _algorithms.push_back(Algorithm("Edge Detection", "edgedetect"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("int", "countValue");

  // Fault Repair.
  _algorithms.push_back(Algorithm("Fault Repair", "faultrepair"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "holeProb");

  // Hole Elimination.
  _algorithms.push_back(Algorithm("Hole Elimination", "holeelimination"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "holeProb");

  // Infinite Object Coating.
  _algorithms.push_back(Algorithm("Infinite Object Coating", "infobjcoating"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "holeProb");

  // Ising Model.
  _algorithms.push_back(Algorithm("Ising Model", "ising"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "beta");

  // Line Formation.
  _algorithms.push_back(Algorithm("Line Formation", "line"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "holeProb");

  // Line Sorting. (TODO)
  _algorithms.push_back(Algorithm("Line Sorting", "linesort"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "holeProb");

  // Matrix. (TODO)
  _algorithms.push_back(Algorithm("Matrix", "matrix"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("int", "countValue");
  _algorithms.back().addParameter("int", "whichStream");
  _algorithms.back().addParameter("int", "mode");

  // Rectangle. (TODO)
  _algorithms.push_back(Algorithm("Rectangle", "rectangle"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "holeProb");

  // Ring Formation.
  _algorithms.push_back(Algorithm("Ring Formation", "ring"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "holeProb");

  // Basic Shape Formation.
  _algorithms.push_back(Algorithm("Basic Shape Formation", "shapeformation"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "holeProb");
  _algorithms.back().addParameter("QString", "mode");

  // Sierpinski. (TODO)
  _algorithms.push_back(Algorithm("Sierpinski", "sierpinski"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "holeProb");

  // Swarm Separation. (TODO)
  _algorithms.push_back(Algorithm("Swarm Separation", "swarmseparation"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "c_rand");
  _algorithms.back().addParameter("double", "c_repulse");

  // 2-Site Connected Bridging.
  _algorithms.push_back(Algorithm("2-Site C-Bridging", "twositecbridge"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "lambda");
  _algorithms.back().addParameter("double", "alpha");

  // 2-Site Exploration Bridging.
  _algorithms.push_back(Algorithm("2-Site E-Bridging", "twositeebridge"));
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "explambda");
  _algorithms.back().addParameter("double", "complambda");
  _algorithms.back().addParameter("double", "siteDistance");

  /* LEGACY ALGORITHMS */

  // Bounded Object Coating.
  _algorithms.push_back(Algorithm("Bounded Object Coating", "boundedobjcoating"));
  _algorithms.back().addParameter("int", "numStaticParticles");
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "holeProb");

  // Leader Election.
  _algorithms.push_back(Algorithm("Leader Election", "leaderelection"));
  _algorithms.back().addParameter("uint", "numParticles");

  // Leader Election Demo.
  _algorithms.push_back(Algorithm("Leader Election Demo", "leaderelectiondemo"));

  // Universal Coating.
  _algorithms.push_back(Algorithm("Universal Coating", "universalcoating"));
  _algorithms.back().addParameter("int", "staticParticlesRadius");
  _algorithms.back().addParameter("int", "numParticles");
  _algorithms.back().addParameter("double", "holeProb");
}

QStringList AlgorithmList::getAlgNames() const {
  QStringList names;
  for (Algorithm alg : _algorithms) {
    names.append(alg.getName());
  }
  names.sort();

  return names;
}

QString AlgorithmList::getAlgSignature(QString algName) const {
  QString signature;
  for (Algorithm alg : _algorithms) {
    if (alg.getName().compare(algName) == 0) {
      signature = alg.getSignature();
    }
  }

  return signature;
}

QStringList AlgorithmList::getParameterNames(QString algName) const {
  QStringList names;
  for (Algorithm alg : _algorithms) {
    if (alg.getName().compare(algName) == 0) {
      names = alg.getParameterNames();
      break;
    }
  }

  return names;
}
