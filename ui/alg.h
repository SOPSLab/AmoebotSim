#ifndef AMOEBOTSIM_UI_ALGORITHM_H_
#define AMOEBOTSIM_UI_ALGORITHM_H_

#include <utility>
#include <vector>

#include <QString>
#include <QStringList>

class Algorithm {
 public:
  // Constructs an algorithm with a given name (e.g., "Infinite Object Coating")
  // and a given signature (e.g., "infobjcoating").
  Algorithm(QString name, QString signature);

  // Typical getters and setters for the algorithm name and signature.
  QString getName() const;
  void setName(QString name);
  QString getSignature() const;
  void setSignature(QString signature);

  // Adds a parameter to the algorithm of the given type and name.
  void addParameter(QString type, QString parameter);

 private:
  QString _name;
  QString _signature;
  std::vector<std::pair<QString, QString>> _parameters;
};

class AlgorithmList {
 public:
  // Constructs a list of algorithms recognized by the simulator (for now, this
  // mirrors ScriptInterface).
  AlgorithmList();

  // Returns a list of all the algorithm's names in this list.
  QStringList getAlgNames() const;

 private:
  std::vector<Algorithm> _algorithms;
};

#endif  // AMOEBOTSIM_UI_ALGORITHM_H_
