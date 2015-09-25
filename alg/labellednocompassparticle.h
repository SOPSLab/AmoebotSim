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

    // directions are by default assumed to be local
    // global directions are marked as such
    // labels are, of course, always local

    // local directions and labels
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

    // these include some form of global information
    int labelToGlobalDir(int label) const;
    int labelOfNeighboringNodeInGlobalDir(const Node& node, int globalDir) const;

    Node occupiedNodeIncidentToLabel(int label) const;
    Node neighboringNodeReachedViaLabel(int label) const;

    // some helpers
    int localToGlobalDir(int dir) const;
    int globalToLocalDir(int globalDir) const;

    int neighborDirToDir(const LabelledNoCompassParticle& neighbor, int neighborDir) const;
    int dirToNeighborDir(const LabelledNoCompassParticle& neighbor, int myDir) const;

    bool pointsAtMe(const LabelledNoCompassParticle& neighbor, int neighborLabel) const;
    bool pointsAtMyHead(const LabelledNoCompassParticle& neighbor, int neighborLabel) const;
    bool pointsAtMyTail(const LabelledNoCompassParticle& neighbor, int neighborLabel) const;

public:
    const int orientation; // global direction

private:
    static const std::vector<int> sixLabels;
    static const std::array<const std::vector<int>, 6> labels;
    static const std::array<int, 6> contractLabels;
    static const std::array<std::array<int, 10>, 6> labelDir;
};

#endif // LABELLEDNOCOMPASSPARTICLE_H
