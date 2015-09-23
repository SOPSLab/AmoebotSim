#ifndef LABELLEDNOCOMPASSPARTICLE_H
#define LABELLEDNOCOMPASSPARTICLE_H

#include <array>
#include <vector>

#include "sim/node.h"
#include "sim/particle.h"

class LabelledNoCompassParticle : public Particle
{
public:
    LabelledNoCompassParticle(const Node head,
                     const int globalTailDir,
                     const int _orientation);
    virtual ~LabelledNoCompassParticle();

    // directions are by default assumed to be local
    // global directions are marked as such
    // labels are, of course, always local

    // local directions and labels
    int tailDir() const;

    int labelToDir(const int label) const;
    int labelToDirAfterExpansion(const int label, const int expansionDir) const;

    const std::vector<int>& headLabels() const;
    const std::vector<int>& tailLabels() const;
    bool isHeadLabel(const int label) const;
    bool isTailLabel(const int label) const;
    int headContractionLabel() const;
    int tailContractionLabel() const;

    const std::vector<int>& headLabelsAfterExpansion(const int expansionDir) const;
    const std::vector<int>& tailLabelsAfterExpansion(const int expansionDir) const;
    bool isHeadLabelAfterExpansion(const int label, const int expansionDir) const;
    bool isTailLabelAfterExpansion(const int label, const int expansionDir) const;
    int dirToHeadLabelAfterExpansion(const int dir, const int expansionDir) const;
    int dirToTailLabelAfterExpansion(const int dir, const int expansionDir) const;
    int headContractionLabelAfterExpansion(const int expansionDir) const;
    int tailContractionLabelAfterExpansion(const int expansionDir) const;

    // these include some form of global information
    int labelToGlobalDir(const int label) const;
    int labelOfNeighboringNodeInGlobalDir(const Node node, const int globalDir) const;

    Node occupiedNodeIncidentToLabel(const int label) const;
    Node neighboringNodeReachedViaLabel(const int label) const;

    // some helpers
    int localToGlobalDir(int dir) const;
    int globalToLocalDir(int globalDir) const;

    template<int n> static int posMod(const int a);

public:
    int orientation; // global direction

private:
    static const std::vector<int> sixLabels;
    static const std::array<const std::vector<int>, 6> labels;
    static const std::array<int, 6> contractLabels;
    static const std::array<std::array<int, 10>, 6> labelDir;
};

template<int n> int LabelledNoCompassParticle::posMod(const int a)
{
    return (a % n + n) % n;
}

#endif // LABELLEDNOCOMPASSPARTICLE_H
