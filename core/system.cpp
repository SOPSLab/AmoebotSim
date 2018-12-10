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

// TODO: remove?
System::System() {}
System::~System() {}

SystemIterator System::begin() const {
  return SystemIterator(this, 0);
}

SystemIterator System::end() const {
  return SystemIterator(this, size());
}

unsigned int System::numMovements() const {
  return 0;
}

unsigned int System::numRounds() const {
  return 0;
}

unsigned int System::leaderElectionRounds() const {
  return 0;
}

unsigned int System::weakBounds() const {
  return 0;
}

unsigned int System::strongBounds() const {
  return 1;
}

bool System::hasTerminated() const {
  return false;
}
