#ifndef LEGACYPARTICLE_H
#define LEGACYPARTICLE_H

#include <array>
#include <memory>
#include <vector>

#include "alg/legacy/algorithm.h"
#include "alg/legacy/movement.h"
#include "sim/node.h"
#include "sim/particle.h"

class LegacyParticle : public Particle
{
public:
    LegacyParticle(std::shared_ptr<Algorithm> _algorithm,
                   const int _orientation,
                   const Node _head,
                   const int _tailDir = -1);
    virtual ~LegacyParticle();

    Movement executeAlgorithm(std::array<const Flag*, 10>& inFlag);

    void apply();
    void discard();

    const Flag* getFlagForNodeInDir(const Node node, const int dir);
    std::shared_ptr<const Algorithm> getAlgorithm() const;

    bool algorithmIsDeterministic() const;

    bool isStatic() const;

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

    virtual int headMarkColor() const;
    virtual int headMarkDir() const;
    virtual int tailMarkColor() const;
    virtual int tailMarkDir() const;

    virtual std::array<int, 18> borderColors() const;
    virtual int borderDir(const int dir) const;
    virtual std::array<int, 6> borderPointColors() const;
    virtual int borderPointDir(const int dir) const;

    template<int n> static int posMod(const int a);

public:
    int orientation; // global direction

protected:
    std::shared_ptr<Algorithm> algorithm;
    std::shared_ptr<Algorithm> newAlgorithm;

private:
    static const std::vector<int> sixLabels;
    static const std::array<const std::vector<int>, 6> labels;
    static const std::array<int, 6> contractLabels;
    static const std::array<std::array<int, 10>, 6> labelDir;
};

template<int n> int LegacyParticle::posMod(const int a)
{
    return (a % n + n) % n;
}

#endif // LEGACYPARTICLE_H
