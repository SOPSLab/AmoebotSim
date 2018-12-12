// Defines an object, which is a single node of a solid object.

#ifndef AMOEBOTSIM_SIM_OBJECT_H_
#define AMOEBOTSIM_SIM_OBJECT_H_

#include "core/node.h"

class Object {
 public:
  // Constructs an Object entity positioned at the given Node
  Object(const Node& _node = Node());

  Node _node;
};

#endif  // AMOEBOTSIM_SIM_OBJECT_H_
