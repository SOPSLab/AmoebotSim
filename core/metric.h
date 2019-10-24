// Defines a class to capture a numerical metric of a particle
// system during an algorithm execution.

#ifndef AMOEBOTSIM_SIM_METRIC_H
#define AMOEBOTSIM_SIM_METRIC_H

#include <vector>

#include "core/amoebotsystem.h"

class Measure {
 public:

  // Default constructor and deconstructor for the measure class.
  Measure();
  virtual ~Measure();

  // Virtual function to be overwritten to calculate the desired metric.
  virtual void calculate(AmoebotSystem system) = 0;

  // Frequency determines how often the metric is calculated during the
  // algorithm execution. A value of i would be executed every i round.
  int frequency;

  // Vector to capture the value of the metric over time.
  std::vector<double> history;
};

class Count {
  public:

    // Default constructor and deconstructor for the count class.
    Count();
    virtual ~Count();

    // Virtual function to be overwritten to calculate the desired metric upon call
    virtual void registerCount(AmoebotSystem System) = 0;

    // The name will hold the actual property which will be observed in this metric
    // Value will represent the most latest occurrence of the metric
    std::string name;
    int value;

    // Vector to capture the value of the metric over time.
    std::vector<double> history;
};

#endif // AMOEBOTSIM_SIM_METRIC_H
