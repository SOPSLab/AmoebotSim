// Defines a tile, which is a single node of a solid object.

#ifndef AMOEBOTSIM_SIM_TILE_H
#define AMOEBOTSIM_SIM_TILE_H

#include "sim/node.h"

class Tile {
 public:
  // Constructs a new particle with a node position for its head and a global
  // compass direction from its head to its tail (-1 if contracted).
  Tile(const Node& node = Node());

  Node node;
};

#endif  // AMOEBOTSIM_SIM_TILE_H
