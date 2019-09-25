#include "ui/alg.h"

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

AlgorithmList::AlgorithmList() {
  /* DEMO ALGORITHMS */

  // Demo: Disco, a first tutorial.
  _algorithms.push_back(new Algorithm("Demo: Disco", "discodemo"));
  _algorithms.back()->addParameter("# Particles", "30");
  _algorithms.back()->addParameter("Counter Max", "5");

  // Demo: Pull Handovers.
  _algorithms.push_back(new Algorithm("Demo: Pull Handovers", "pulldemo"));

  // Demo: Token Passing.
  _algorithms.push_back(new Algorithm("Demo: Token Passing", "tokendemo"));
  _algorithms.back()->addParameter("# Particles", "200");
  _algorithms.back()->addParameter("Hole Prob.", "0.2");

  /* ALGORITHMS */

  // Compression.
  _algorithms.push_back(new Algorithm("Compression", "compression"));
  _algorithms.back()->addParameter("# Particles", "100");
  _algorithms.back()->addParameter("Lambda", "4.0");

  // Infinite Object Coating.
  _algorithms.push_back(new Algorithm("Infinite Object Coating", "infobjcoating"));
  _algorithms.back()->addParameter("# Particles", "100");
  _algorithms.back()->addParameter("Hole Prob.", "0.2");

  // Basic Shape Formation.
  _algorithms.push_back(new Algorithm("Basic Shape Formation", "shapeformation"));
  _algorithms.back()->addParameter("# Particles", "200");
  _algorithms.back()->addParameter("Hole Prob.", "0.2");
  _algorithms.back()->addParameter("Shape", "\"h\"");

  /* LEGACY ALGORITHMS */

  // Bounded Object Coating.
  _algorithms.push_back(new Algorithm("Bounded Object Coating", "boundedobjcoating"));
  _algorithms.back()->addParameter("# Objects", "100");
  _algorithms.back()->addParameter("# Particles", "50");
  _algorithms.back()->addParameter("Hole Prob.", "0.2");

  // Leader Election.
  _algorithms.push_back(new Algorithm("Leader Election", "leaderelection"));
  _algorithms.back()->addParameter("# Particles", "100");
  _algorithms.back()->addParameter("Hole Prob.", "0.2");

  // Universal Coating.
  _algorithms.push_back(new Algorithm("Universal Coating", "universalcoating"));
  _algorithms.back()->addParameter("Object Radius", "5");
  _algorithms.back()->addParameter("# Particles", "50");
  _algorithms.back()->addParameter("Hole Prob.", "0.2");
}

AlgorithmList::~AlgorithmList() {
  _algorithms.erase(_algorithms.begin(), _algorithms.end());
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
