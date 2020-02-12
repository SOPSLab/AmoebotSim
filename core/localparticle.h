/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines a particle which is limited to local information and local compass
// directions, and implements these directions via port (edge) labels.
//
// Some notes on terminology:
//   Direction: # in {0,...,5} that represents a vector in the Euclidean plane.
//   Global Dir: the global compass version of direction; global direction 0
//               corresponds to the vector pointing right in the triangular
//               lattice, and the other global directions increase clockwise.
//   Local Dir: the local compass version of direction; each particle has a
//              (random) orientation in {0,...,5}, and local direction is
//              calculated as localDir = (globalDir - orientation + 6) % 6.
//   Label: # in {0,...,9} representing a particle's name for an edge (not node)
//          between it and a neighboring node not occupied by the particle. For
//          a contracted particle, the edge pointing in local direction 0 gets
//          label 0. This is also true for an expanded particle, except there
//          may be two edges pointing in local direction 0. In this case, the
//          edge pointing "away" from the particle (i.e., to a node adjacent to
//          only one of the two nodes occupied by the particle) gets label 0.
//          From label 0, labels increase counter-clockwise around the particle.

#ifndef AMOEBOTSIM_CORE_LOCALPARTICLE_H_
#define AMOEBOTSIM_CORE_LOCALPARTICLE_H_

#include <array>
#include <vector>

#include "core/node.h"
#include "core/particle.h"

class LocalParticle : public Particle {
 public:
  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), and an
  // offset for its local compass.
  LocalParticle(const Node& head, int globalTailDir, const int orientation);

  // Returns the local direction of the tail relative to the head, (-1 if
  // contracted).
  int tailDir() const;

  // Functions for converting port (edge) labels to local directions. labelToDir
  // returns the local direction the edge with the given label points to.
  // labelToDirAfterExpansion returns the local direction the edge with the
  // given label would point to after an expansion in the given local direction.
  int labelToDir(int label) const;
  int labelToDirAfterExpansion(int label, int expansionDir) const;

  // Returns a list of labels which uniquely address the neighboring nodes.
  const std::vector<int> uniqueLabels() const;

  // Functions for accessing labels specifically indicent to the head or tail.
  // headLabels (respectively, tailLabels) returns a vector of labels of edges
  // incident to the particle's head (respectively, tail). isHeadLabel (resp.,
  // isTailLabel) checks whether the given label is a head (resp., tail) label.
  // dirToHeadLabel (resp., dirToTailLabel) returns the head (resp., tail) label
  // of the edge pointing in the given local direction. These conversions will
  // fail on an expanded particle if dirToHeadLabel (resp., dirToTailLabel) is
  // called with the local direction from head to tail (resp., tail to head), as
  // the edge connecting the head and tail is not labelled. headContractionLabel
  // (resp., tailContractionLabel) returns the label needed to perform a head
  // (resp., tail) contraction.
  const std::vector<int>& headLabels() const;
  const std::vector<int>& tailLabels() const;
  bool isHeadLabel(int label) const;
  bool isTailLabel(int label) const;
  int dirToHeadLabel(int dir) const;
  int dirToTailLabel(int dir) const;
  int headContractionLabel() const;
  int tailContractionLabel() const;

  // Functions analogous to their non -AfterExpansion versions above, but return
  // values as if the particle first expanded in the given local direction.
  const std::vector<int>& headLabelsAfterExpansion(int expansionDir) const;
  const std::vector<int>& tailLabelsAfterExpansion(int expansionDir) const;
  bool isHeadLabelAfterExpansion(int label, int expansionDir) const;
  bool isTailLabelAfterExpansion(int label, int expansionDir) const;
  int dirToHeadLabelAfterExpansion(int dir, int expansionDir) const;
  int dirToTailLabelAfterExpansion(int dir, int expansionDir) const;
  int headContractionLabelAfterExpansion(int expansionDir) const;
  int tailContractionLabelAfterExpansion(int expansionDir) const;

  // Helper functions which include some level of global information.

  // labelToGlobalDir returns the global direction the edge with the given label
  // points to. labelOfNbrNodeInGlobalDir returns the label of the edge that
  // connects the particle to given node (fails if the specified node is not in
  // the particle's neighborhood).
  int labelToGlobalDir(int label) const;
  int labelOfNbrNodeInGlobalDir(const Node& node, int globalDir) const;

  // occupiedNodeIncidentToLabel returns the head node if the given label is a
  // head label and the tail node otherwise. nbrNodeReachedViaLabel returns the
  // node reached from the particle by the edge with the given label.
  Node occupiedNodeIncidentToLabel(int label) const;
  Node nbrNodeReachedViaLabel(int label) const;

  // Functions for converting between local and global compass directions.
  int localToGlobalDir(int localDir) const;
  int globalToLocalDir(int globalDir) const;

  // Functions for converting between neighboring particles' local compasses.
  // nbrDirToDir returns the local direction from this particle's compass which
  // points in the same global direction as the given local direction from the
  // neighbor's compass. dirToNbrDir returns the local direction from the
  // neighbor's compass which points in the same global direction as the given
  // local direction from this particle's compass.
  int nbrDirToDir(const LocalParticle& nbr, int nbrDir) const;
  int dirToNbrDir(const LocalParticle& nbr, int myDir) const;

  // Functions for determining if a neighbor's label refers to an edge incident
  // to this particle. pointsAtMe checks if the labelled edge is incident
  // anywhere on this particle, whereas pointsAtMyHead (resp., pointsAtMyTail)
  // checks if the labelled edge is incident specifically to this particle's
  // head (resp., tail).
  bool pointsAtMe(const LocalParticle& nbr, int nbrLabel) const;
  bool pointsAtMyHead(const LocalParticle& nbr, int nbrLabel) const;
  bool pointsAtMyTail(const LocalParticle& nbr, int nbrLabel) const;

  const int orientation;  // Offset from global direction for local compass.

 private:
  static const std::vector<int> sixLabels;
  static const std::array<const std::vector<int>, 6> labels;
  static const std::array<int, 6> contractLabels;
  static const std::array<std::array<int, 10>, 6> labelDir;
};

#endif  // AMOEBOTSIM_CORE_LOCALPARTICLE_H_
