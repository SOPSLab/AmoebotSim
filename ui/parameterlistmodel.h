#ifndef AMOEBOTSIM_UI_PARAMETERLISTMODEL_H_
#define AMOEBOTSIM_UI_PARAMETERLISTMODEL_H_

#include <QStringListModel>

#include "ui/alg.h"

class ParameterListModel : public QStringListModel {
  Q_OBJECT

 public:
  enum roleTypes {
    PARAM_NAME = Qt::UserRole + 1,
    PARAM_COUNT
  };

  // Constructs a parameter list model with a reference to the algorithms list.
  ParameterListModel(QObject* parent = nullptr, AlgorithmList* algs = nullptr);

  // Set the parameter list model's algorithm list.
  void setAlgorithmList(AlgorithmList* algs);

  // Overrides to return the parameter name at the given index as a QVariant.
  QHash<int, QByteArray> roleNames() const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

 public slots:
  // Updates this model's internal string list to be the list of parameter names
  // for the given algorithm.
  void updateAlgParameters(QString algName);

 private:
  AlgorithmList * _algs;
};

#endif  // AMOEBOTSIM_UI_PARAMETERLISTMODEL_H_
