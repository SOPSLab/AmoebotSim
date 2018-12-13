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
  roles[PARAM_VALUE] = "parameterValue";

  return roles;
}

QVariant ParameterListModel::data(const QModelIndex &index, int role) const {
  if (role == PARAM_NAME) {
    if (index.row() < 0 || index.row() >= rowCount()) {
      return QVariant();
    } else {
      return QVariant::fromValue(stringList()[index.row()]);
    }
  }

  return QVariant();
}

bool ParameterListModel::setData(const QModelIndex &index,
                                 const QVariant &value, int role) {
  if (role == PARAM_VALUE) {
    if (index.row() < 0 || index.row() >= rowCount()) {
      return false;
    } else {
      _values[index.row()] = value.toString();
      return true;
    }
  } else {
    return QStringListModel::setData(index, value, role);
  }
}

void ParameterListModel::updateAlgParameters(QString algName) {
  setStringList(_algs->getParameterNames(algName));
  _values.clear();
  for (int i = 0; i < rowCount(); ++i) {
    _values << "";
  }
}

void ParameterListModel::createCommand(QString algName) {
  QStringList defaults = _algs->getParameterDefaults(algName);

  QString cmd;
  cmd += _algs->getAlgSignature(algName) + "(";
  for (int i = 0; i < _values.size(); ++i) {
    if (_values[i].compare("") != 0) {
      cmd += _values[i];
    } else {
      cmd += defaults[i];
    }

    if (i + 1 < _values.size()) {
      cmd += ", ";
    }
  }
  cmd += ")";

  emit executeCommand(cmd);
}
