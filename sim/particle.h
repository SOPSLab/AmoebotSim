#ifndef PARTICLE_H
#define PARTICLE_H

#include <array>
#include <vector>
#include <memory>

#include "alg/legacy/algorithm.h"
#include "sim/movement.h"
#include "sim/node.h"

class Particle
{
public:
    Particle(std::shared_ptr<Algorithm> _algorithm, const int _orientation, const Node _head, const int _tailDir = -1);
    Particle(const Particle& other);
    virtual ~Particle();

    Particle& operator=(const Particle& other);

    Movement executeAlgorithm(std::array<const Flag*, 10>& inFlag);
    void apply();
    void discard();

    const Flag* getFlagForNodeInDir(const Node node, const int dir);
    std::shared_ptr<const Algorithm> getAlgorithm() const;

    Node tail() const;

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

    int headMarkColor() const;
    int headMarkDir() const;
    int tailMarkColor() const;
    int tailMarkDir() const;
    std::array<int, 18> borderColors() const;
    int borderDir(const int dir) const;
    std::array<int, 6> borderPointColors() const;
    int borderPointDir(const int dir) const;
    bool algorithmIsDeterministic() const;
    bool isStatic() const;

    template<int n> static int posMod(const int a);

public:
    int orientation; // global direction
    Node head;
    int tailDir; // global direction

protected:
    std::shared_ptr<Algorithm> algorithm;
    std::shared_ptr<Algorithm> newAlgorithm;

private:
    static const std::vector<int> sixLabels;
    static const std::array<const std::vector<int>, 6> labels;
    static const std::array<int, 6> contractLabels;
    static const std::array<std::array<int, 10>, 6> labelDir;
};

template<int n> int Particle::posMod(const int a)
{
    return (a % n + n) % n;
}

#endif // PARTICLE_H
