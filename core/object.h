// Defines a object, which is a single node of a solid object.

#ifndef AMOEBOTSIM_SIM_OBJECT_H
#define AMOEBOTSIM_SIM_OBJECT_H

#include "core/node.h"

class Object {
 public:
  // Constructs a new particle with a node position for its head and a global
  // compass direction from its head to its tail (-1 if contracted).
  Object(const Node& node = Node());

  Node node;
};

#endif  // AMOEBOTSIM_SIM_OBJECT_H
