// Defines a class to capture a numerical metric of a particle
// system during an algorithm execution.

#ifndef AMOEBOTSIM_SIM_METRIC_H
#define AMOEBOTSIM_SIM_METRIC_H

#include <vector>

#include "core/amoebotsystem.h"

class Metric {
 public:

  // Default constructor and deconstructor for the metric class.
  Metric();
  virtual ~Metric();

  // Virtual function to be overwritten to calculate the desired metric.
  virtual void calculate(AmoebotSystem system) = 0;

  // Frequency determines how often the metric is calculated during the
  // algorithm execution. A value of i would be executed every i round.
  int frequency;

  // Vector to capture the value of the metric over time.
  std::vector<double> history;
};

#endif // AMOEBOTSIM_SIM_METRIC_H
