/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#ifndef AMOEBOTSIM_UI_PARAMETERLISTMODEL_H_
#define AMOEBOTSIM_UI_PARAMETERLISTMODEL_H_

#include <QStringListModel>

#include "ui/algorithm.h"

class ParameterListModel : public QStringListModel {
  Q_OBJECT

 public:
  enum roleTypes {
    PARAM_NAME = Qt::UserRole + 1,
    PARAM_VALUE
  };

  // Constructs a parameter list model with a reference to the algorithms list.
  ParameterListModel(QObject* parent = nullptr);

  // Destructs the algorithm list this parameter list model contains.
  virtual ~ParameterListModel();

  // Get the parameter list model's algorithm list.
  AlgorithmList * getAlgorithmList();

  // Overrides to return the parameter name at the given index as a QVariant and
  // set the parameter value at the given index.
  QHash<int, QByteArray> roleNames() const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole);

 signals:
  void executeCommand(QString cmd);

 public slots:
  // Updates this model's internal string list to be the list of parameter names
  // for the given algorithm.
  void updateAlgParameters(QString algName);

  // Creates a command string using the current parameter values and emits it as
  // a signal to be processed by the script engine.
  void createCommand(QString algName);

 private:
  AlgorithmList * _algs;
  QStringList _values;
};

#endif  // AMOEBOTSIM_UI_PARAMETERLISTMODEL_H_
