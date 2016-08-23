#ifndef LABELLEDNOCOMPASSPARTICLE_H
#define LABELLEDNOCOMPASSPARTICLE_H

#include <array>
#include <vector>

#include "sim/node.h"
#include "sim/particle.h"

class LabelledNoCompassParticle : public Particle
{
public:
    LabelledNoCompassParticle(const Node& head, int globalTailDir, int orientation);

    /* Some notes on terminology for better understanding of these functions:
     * Direction: # in [0,5] that represents a vector in the Euclidean plane.
     *     Global dir: the compass version of direction; global dir 0 corresponds
     *                 with the vector pointing right in the hexagonal grid, and
     *                 the other global dirs increase clockwise from global dir 0.
     *     Local dir:  the non-compass version of direction; each particle is
     *                 assigned a (usually random) offset in [0,5], and local dir
     *                 is calculated as: localDir = (globalDir - offset + 6) % 6.
     * Label: # in [0,9] that represents a particle's name for an edge between it
     *        and a neighboring node not occupied by the particle. (IMPORTANT:
     *        edges are being labelled, not nodes). For a contracted particle,
     *        the edge pointing in the 0 local direction gets label 0. This is
     *        also true for an expanded particle, except there may be two edges
     *        pointing in the 0 local direction. In this case, the edge pointing
     *        "away" from the particle--to a node adjacent to only one of the two
     *        nodes occupied by the particle--gets label 0. From label 0, labels
     *        increase counter-clockwise around the particle.
     */

    // direction & label functions that use only local information/compasses
    int tailDir() const;

    int labelToDir(int label) const;
    int labelToDirAfterExpansion(int label, int expansionDir) const;

    const std::vector<int>& headLabels() const;
    const std::vector<int>& tailLabels() const;
    bool isHeadLabel(int label) const;
    bool isTailLabel(int label) const;
    int dirToHeadLabel(int dir) const;
    int dirToTailLabel(int dir) const;
    int headContractionLabel() const;
    int tailContractionLabel() const;

    const std::vector<int>& headLabelsAfterExpansion(int expansionDir) const;
    const std::vector<int>& tailLabelsAfterExpansion(int expansionDir) const;
    bool isHeadLabelAfterExpansion(int label, int expansionDir) const;
    bool isTailLabelAfterExpansion(int label, int expansionDir) const;
    int dirToHeadLabelAfterExpansion(int dir, int expansionDir) const;
    int dirToTailLabelAfterExpansion(int dir, int expansionDir) const;
    int headContractionLabelAfterExpansion(int expansionDir) const;
    int tailContractionLabelAfterExpansion(int expansionDir) const;

    // helper functions that include some form of global information
    int labelToGlobalDir(int label) const;
    int labelOfNeighboringNodeInGlobalDir(const Node& node, int globalDir) const;

    Node occupiedNodeIncidentToLabel(int label) const;
    Node neighboringNodeReachedViaLabel(int label) const;

    int localToGlobalDir(int dir) const;
    int globalToLocalDir(int globalDir) const;

    int neighborDirToDir(const LabelledNoCompassParticle& neighbor, int neighborDir) const;
    int dirToNeighborDir(const LabelledNoCompassParticle& neighbor, int myDir) const;

    bool pointsAtMe(const LabelledNoCompassParticle& neighbor, int neighborLabel) const;
    bool pointsAtMyHead(const LabelledNoCompassParticle& neighbor, int neighborLabel) const;
    bool pointsAtMyTail(const LabelledNoCompassParticle& neighbor, int neighborLabel) const;

public:
    const int orientation; // offset from global direction

private:
    static const std::vector<int> sixLabels;
    static const std::array<const std::vector<int>, 6> labels;
    static const std::array<int, 6> contractLabels;
    static const std::array<std::array<int, 10>, 6> labelDir;
};

#endif // LABELLEDNOCOMPASSPARTICLE_H
