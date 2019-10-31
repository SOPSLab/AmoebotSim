// Defines a class to capture a numerical metric of a particle
// system during an algorithm execution.

#ifndef AMOEBOTSIM_SIM_METRIC_H
#define AMOEBOTSIM_SIM_METRIC_H

#include <string>
#include <vector>

class AmoebotSystem;

class Measure {
 public:

  // Default constructor and deconstructor for the measure class.
  Measure() {}
  virtual ~Measure() {}

  // Virtual function to be overwritten to calculate the desired metric.
  virtual void calculate(AmoebotSystem* system) = 0;

  // Frequency determines how often the measure is calculated during the
  // algorithm execution. A value of i would be executed every i round.
  int frequency;

  // Vector to capture the value of the measure each time it is calculated.
  std::vector<double> history;
};

class Count {
  public:

    // Default constructor and deconstructor for the count class.
    Count() {}
    virtual ~Count() {}

    // Virtual function to be overwritten to calculate the desired metric upon
    // called.
    virtual void record() = 0;

    // The name will hold the name of the property being counted. Value will
    // represent the total value of the current count.
    std::string name;
    int value;

    // Vector to capture the value of the count over each round.
    std::vector<double> history;
};

#endif // AMOEBOTSIM_SIM_METRIC_H
