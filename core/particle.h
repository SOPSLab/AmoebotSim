/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines a base particle which contains the information necessary for drawing
// a particle in the simulator.

#ifndef AMOEBOTSIM_CORE_PARTICLE_H_
#define AMOEBOTSIM_CORE_PARTICLE_H_

#include <array>

#include <QString>

#include "core/node.h"

class Particle {
 public:
  // Constructs a new particle with a node position for its head and a global
  // compass direction from its head to its tail (-1 if contracted).
  Particle(const Node& head = Node(), int globalTailDir = -1);

  // Functions for checking whether the particle is contracted or expanded.
  bool isContracted() const;
  bool isExpanded() const;

  // Returns the node occupied by the particle's tail. Fails if the particle is
  // contracted; consider using isExpanded() first if unsure.
  Node tail() const;

  // Functions for altering the particle's cosmetic appearance. headMarkColor
  // (respectively, tailMarkColor) returns the color to be used for the ring
  // drawn around the head (respectively, tail) node. Tail color is not shown
  // when the particle is contracted. All colors are defined in the 0xrrbbgg
  // format, where -1 indicates no color. headMarkGlobalDir (resp.,
  // tailMarkGlobalDir) returns the global direction from the head (resp., tail)
  // on which to draw the direction marker (-1 indicates no marker).
  virtual int headMarkColor() const;
  virtual int tailMarkColor() const;
  virtual int headMarkGlobalDir() const;
  virtual int tailMarkGlobalDir() const;

  // Functions for altering the cosmetic appearance of the particle's border.
  // borderColors defines the colors for each of the 18 border segment
  // configurations a particle may possibly use. borderPointColors defines the
  // colors used for each of the 6 points on the border a particle may possibly
  // be incident to. All colors are in 0xrrbbgg format; -1 indicates no color.
  virtual std::array<int, 18> borderColors() const;
  virtual std::array<int, 6> borderPointColors() const;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  virtual QString inspectionText() const;

  Node head;
  int globalTailDir;
};

#endif  // AMOEBOTSIM_CORE_PARTICLE_H_
