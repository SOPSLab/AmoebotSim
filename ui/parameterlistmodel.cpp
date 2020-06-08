/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "ui/parameterlistmodel.h"

ParameterListModel::ParameterListModel(QObject* parent)
    : QStringListModel(parent) {
  _algs = new AlgorithmList();
}

ParameterListModel::~ParameterListModel() {
  delete _algs;
}

AlgorithmList * ParameterListModel::getAlgorithmList() {
  return _algs;
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

void ParameterListModel::createSystem(QString algName) {
  QStringList defaults = _algs->getParameterDefaults(algName);

  QString signature;
  std::vector<QString> params;
  Algorithm* alg = _algs->getAlg(algName);
  signature = _algs->getAlgSignature(algName);
  for (int i = 0; i < _values.size(); ++i) {
    if (_values[i].compare("") != 0) {
      params.push_back(_values[i]);
    } else {
      params.push_back(defaults[i]);
    }
  }

  if (signature == "discodemo") {
    dynamic_cast<DiscoDemoAlg*>(alg)->
        instantiate(params[0].toInt(), params[1].toInt());
  } else if (signature == "metricsdemo") {
    dynamic_cast<MetricsDemoAlg*>(alg)->
        instantiate(params[0].toInt(), params[1].toInt());
  } else if (signature == "ballroomdemo") {
    dynamic_cast<BallroomDemoAlg*>(alg)->
        instantiate(params[0].toInt());
  } else if (signature == "tokendemo") {
    dynamic_cast<TokenDemoAlg*>(alg)->
        instantiate(params[0].toInt(), params[1].toInt());
  } else if (signature == "compression") {
    dynamic_cast<CompressionAlg*>(alg)->
        instantiate(params[0].toInt(), params[1].toDouble());
  } else if (signature == "infobjcoating") {
    dynamic_cast<InfObjCoatingAlg*>(alg)->
        instantiate(params[0].toInt(), params[1].toDouble());
  } else if (signature == "shapeformation") {
    dynamic_cast<ShapeFormationAlg*>(alg)->
        instantiate(params[0].toInt(), params[1].toDouble(), params[2]);
  } else if (signature == "leaderelection") {
    dynamic_cast<LeaderElectionAlg*>(alg)->
        instantiate(params[0].toInt(), params[1].toDouble());
  } else {
    // An unrecognized signature has been entered.
    Q_ASSERT(false);
  }
}
