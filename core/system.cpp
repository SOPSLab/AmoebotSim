/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "core/system.h"

SystemIterator::SystemIterator(const System* system, int pos)
  : _pos(pos)
  , system(system) {}

bool SystemIterator::operator!=(const SystemIterator& other) const {
  return _pos != other._pos;
}

const Particle& SystemIterator::operator*() const {
  return system->at(_pos);
}

const SystemIterator& SystemIterator::operator++() {
  ++_pos;
  return *this;
}

SystemIterator System::begin() const {
  return SystemIterator(this, 0);
}

SystemIterator System::end() const {
  return SystemIterator(this, size());
}

bool System::hasTerminated() const {
  return false;
}
