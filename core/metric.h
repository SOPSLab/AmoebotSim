/* Copyright (C) 2019 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines numerical metrics capturing system progress and characteristics
// during algorithm execution.

#ifndef AMOEBOTSIM_CORE_METRIC_H_
#define AMOEBOTSIM_CORE_METRIC_H_

#include <string>
#include <vector>

class AmoebotSystem;

class Count {
 public:
  // Constructs a new count initialized to zero.
  Count(const std::string name);

  // Increments the value of this count by the number of events being recorded,
  // whose default is 1.
  void record(const unsigned int numEvents = 1);

  // Member variables. The count's name should be human-readable, as it is used
  // to represent this count in the GUI. The value of the count is what is
  // incremented. History records the count values over time, once per round.
  const std::string _name;
  unsigned int _value;
  std::vector<int> _history;
};

class Measure {
 public:
  // Constructs a new measure with a given name, calculation frequency, and
  // reference to the system whose property is being measured.
  Measure(const std::string name, const unsigned int freq,
          AmoebotSystem& system);

  // Implements the measurement from the "global" perspective of the
  // AmoebotSystem being measured. Examples: calculate the percentage of
  // particles in a particular state, calculate the perimeter of a system, etc.
  // This is a pure virtual function and must be overridden by child classes.
  virtual double calculate() = 0;

  // Member variables. The measure's name should be human-readable, as it is
  // used to represent this measure in the GUI. Frequency determines how often
  // the measure is calculated in terms of # of rounds. The system reference
  // points to the AmoebotSystem whose property is being measured. History
  // records the measure values over time, once per round.
  const std::string _name;
  const unsigned int _freq;
  AmoebotSystem& _system;
  std::vector<double> _history;
};

#endif  // AMOEBOTSIM_CORE_METRIC_H_
