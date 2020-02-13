/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

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

  // Returns a list of this algorithm's parameter names or default values.
  QStringList getParameterNames() const;
  QStringList getParameterDefaults() const;

  // Adds a parameter to the algorithm of the given name and default value.
  void addParameter(QString parameter, QString defaultValue);

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

  // Destructs the Algorithms that are contained in this list.
  virtual ~AlgorithmList();

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
