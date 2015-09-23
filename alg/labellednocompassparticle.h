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
                     const int tailDir,
                     const int _orientation);
    virtual ~LabelledNoCompassParticle();

    const std::vector<int>& headLabels() const;
    const std::vector<int>& tailLabels() const;
    bool isHeadLabel(const int label) const;
    bool isTailLabel(const int label) const;
    int headContractionLabel() const;
    int tailContractionLabel() const;

    Node occupiedNodeIncidentToLabel(const int label) const;
    Node neighboringNodeReachedViaLabel(const int label) const;
    int labelToDir(const int label) const;
    int labelOfNeighboringNodeInDir(const Node node, const int dir) const;

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
