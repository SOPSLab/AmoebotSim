#ifndef PARTICLE_H
#define PARTICLE_H

#include <array>
#include <vector>

#include "alg/algorithm.h"
#include "sim/movement.h"
#include "sim/node.h"

class Particle
{
public:
    Particle(Algorithm* _algorithm, const int _orientation, const Node _head, const int _tailDir = -1);
    Particle(const Particle& other);
    virtual ~Particle();

    Particle& operator=(const Particle& other);

    Movement executeAlgorithm(std::array<const Flag*, 10>& inFlag);
    void apply();
    void discard();

    const Flag* getFlagForNodeInDir(const Node node, const int dir);

    Node tail() const;

    const std::vector<int>& headLabels() const;
    const std::vector<int>& tailLabels() const;
    bool isHeadLabel(const int label) const;
    bool isTailLabel(const int label) const;

    Node occupiedNodeIncidentToLabel(const int label) const;
    Node neighboringNodeReachedViaLabel(const int label) const;
    int labelToDir(const int label) const;
    int labelOfNeighboringNodeInDir(const Node node, const int dir) const;

    int headMarkColor() const;
    int headMarkDir() const;
    int tailMarkColor() const;
    int tailMarkDir() const;
    bool algorithmIsDeterministic() const;

    template<int n> static int posMod(const int a);

public:
    int orientation; // global direction
    Node head;
    int tailDir; // global direction

protected:
    Algorithm* algorithm;
    Algorithm* newAlgorithm;

private:
    static const std::vector<int> sixLabels;
    static const std::array<const std::vector<int>, 6> labels;
    static const std::array<std::array<int, 10>, 6> labelDir;
};

template<int n> int Particle::posMod(const int a)
{
    return (a % n + n) % n;
}

#endif // PARTICLE_H
