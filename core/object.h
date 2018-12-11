// Defines a object, which is a single node of a solid object.
//
// NOTE: Objects do not count as particles, and as such, methods such as
//       nbrAtLabel and labelOfFirstNbrWithProperty will not detect Objects.
//       See accompanying methods (hasObjectAtLabel and labelOfFirstObjectNbr)
//       for ways to detect Object entities.

#ifndef AMOEBOTSIM_SIM_OBJECT_H
#define AMOEBOTSIM_SIM_OBJECT_H

#include "core/node.h"

class Object {
 public:
  // Constructs an Object entity positioned at the given Node
  Object(const Node& node = Node());

  Node node;
};

#endif  // AMOEBOTSIM_SIM_OBJECT_H
