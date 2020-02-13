/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "core/metric.h"

#include "core/amoebotsystem.h"

Count::Count(const QString name)
  : _name(name),
    _value(0) {}

void Count::record(const unsigned int numEvents) {
  _value += numEvents;
}

Measure::Measure(const QString name, const unsigned int freq)
  : _name(name),
    _freq(freq) {}

Measure::~Measure() {}
