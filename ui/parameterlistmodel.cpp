#include "ui/parameterlistmodel.h"

ParameterListModel::ParameterListModel(QObject* parent, AlgorithmList* algs) :
  QStringListModel(parent),
  _algs(algs) {}

void ParameterListModel::setAlgorithmList(AlgorithmList* algs) {
  _algs = algs;
}

QHash<int, QByteArray> ParameterListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[PARAM_NAME] = "parameterName";
  roles[PARAM_COUNT] = "parameterCount";

  return roles;
}

QVariant ParameterListModel::data(const QModelIndex &index, int role) const {
  if (role == PARAM_NAME) {
    if (index.row() < 0 || index.row() >= rowCount()) {
      return QVariant();
    } else {
      return QVariant::fromValue(stringList()[index.row()]);
    }
  } else if (role == PARAM_COUNT) {
    return QVariant::fromValue(rowCount());
  }

  return QVariant();
}

void ParameterListModel::updateAlgParameters(QString algName) {
  setStringList(_algs->getParameterNames(algName));
}
