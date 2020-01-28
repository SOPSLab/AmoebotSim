/* Copyright (C) 2019 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "core/metric.h"

Count::Count(const std::string name)
  : _name(name),
    _value(0) {}

void Count::record(const unsigned int numEvents) {
  _value += numEvents;
}

Measure::Measure(const std::string name, const unsigned int freq,
                 AmoebotSystem& system)
  : _name(name),
    _freq(freq),
    _system(system) {}
