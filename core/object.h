/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines an object, which is a single node of a solid object.

#ifndef AMOEBOTSIM_CORE_OBJECT_H_
#define AMOEBOTSIM_CORE_OBJECT_H_

#include "core/node.h"

class Object {
 public:
  // Constructs an Object entity positioned at the given Node
  Object(const Node& _node = Node());

  Node _node;
};

#endif  // AMOEBOTSIM_CORE_OBJECT_H_
