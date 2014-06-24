#include <QtGlobal>

#include "sim/particle.h"

const std::vector<int> Particle::sixLabels = {{0, 1, 2, 3, 4, 5}};
const std::array<const std::vector<int>, 6> Particle::labels =
{{
    {3, 4, 5, 6, 7},
    {4, 5, 6, 7, 8},
    {7, 8, 9, 0, 1},
    {8, 9, 0, 1, 2},
    {9, 0, 1, 2, 3},
    {2, 3, 4, 5, 6}
}};
const std::array<int, 6> Particle::contractLabels = {{0, 1, 4, 5, 6, 9}};
const std::array<std::array<int, 10>, 6> Particle::labelDir
{{
    {{0, 1, 2, 1, 2, 3, 4, 5, 4, 5}},
    {{0, 1, 2, 3, 2, 3, 4, 5, 0, 5}},
    {{0, 1, 0, 1, 2, 3, 4, 3, 4, 5}},
    {{0, 1, 2, 1, 2, 3, 4, 5, 4, 5}},
    {{0, 1, 2, 3, 2, 3, 4, 5, 0, 5}},
    {{0, 1, 0, 1, 2, 3, 4, 3, 4, 5}}
}};

Particle::Particle(Algorithm* _algorithm, const int _orientation, const Node _head, const int _tailDir)
    : orientation(_orientation),
      head(_head),
      tailDir(_tailDir),
      algorithm(_algorithm),
      newAlgorithm(nullptr)
{
    Q_ASSERT(_algorithm != nullptr);
}

Particle::Particle(const Particle& other)
    : orientation(other.orientation),
      head(other.head),
      tailDir(other.tailDir),
      algorithm(other.algorithm->clone()),
      newAlgorithm(nullptr)
{
}

Particle::~Particle()
{
    delete algorithm;
    delete newAlgorithm;
}

Particle& Particle::operator=(const Particle& other)
{
    delete algorithm;
    orientation = other.orientation;
    head = other.head;
    tailDir = other.tailDir;
    algorithm = other.algorithm->clone();
    return *this;
}

Movement Particle::executeAlgorithm(std::array<const Flag*, 10>& inFlag)
{
    Q_ASSERT(newAlgorithm == nullptr);
    newAlgorithm = algorithm->clone();
    return newAlgorithm->delegateExecute(inFlag);
}

void Particle::apply()
{
    Q_ASSERT(newAlgorithm != nullptr);
    delete algorithm;
    algorithm = newAlgorithm;
    newAlgorithm = nullptr;
}

void Particle::discard()
{
    Q_ASSERT(newAlgorithm != nullptr);
    delete newAlgorithm;
    newAlgorithm = nullptr;
}

const Flag* Particle::getFlagForNodeInDir(const Node node, const int dir)
{
    return algorithm->outFlags[labelOfNeighboringNodeInDir(node, dir)];
}

Node Particle::tail() const
{
    return head.nodeInDir(tailDir);
}

const std::vector<int>& Particle::headLabels() const
{
    Q_ASSERT(-1 <= tailDir && tailDir < 6);
    if(tailDir == -1) {
        return sixLabels;
    } else {
        int relativeTailDir = (tailDir - orientation + 6) % 6;
        return labels[relativeTailDir];
    }
}

const std::vector<int>& Particle::tailLabels() const
{
    Q_ASSERT(-1 <= tailDir && tailDir < 6);
    if(tailDir == -1) {
        return sixLabels;
    } else {
        int relativeTailDir = (tailDir - orientation + 6) % 6;
        return labels[(relativeTailDir + 3) % 6];
    }
}

bool Particle::isHeadLabel(const int label) const
{
    for(auto it = headLabels().cbegin(); it != headLabels().cend(); ++it) {
        if(label == *it) {
            return true;
        }
    }
    return false;
}

bool Particle::isTailLabel(const int label) const
{
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        if(label == *it) {
            return true;
        }
    }
    return false;
}

int Particle::headContractionLabel() const
{
    Q_ASSERT(0 <= tailDir && tailDir <= 5);
    int relativeTailDir = (tailDir - orientation + 6) % 6;
    return contractLabels[relativeTailDir];
}

int Particle::tailContractionLabel() const
{
    Q_ASSERT(0 <= tailDir && tailDir <= 5);
    int relativeTailDir = (tailDir - orientation + 6) % 6;
    return contractLabels[(relativeTailDir + 3) % 6];
}

Node Particle::occupiedNodeIncidentToLabel(const int label) const
{
    Q_ASSERT(-1 <= tailDir && tailDir < 6);
    if(tailDir == -1) {
        Q_ASSERT(0 <= label && label < 6);
        return head;
    } else {
        Q_ASSERT(0 <= label && label < 10);
        if(isHeadLabel(label)) {
            return head;
        } else {
            return tail();
        }
    }
}

Node Particle::neighboringNodeReachedViaLabel(const int label) const
{
    Q_ASSERT(-1 <= tailDir && tailDir < 6);
    if(tailDir == -1) {
        Q_ASSERT(0 <= label && label < 6);
        return head.nodeInDir(posMod<6>(orientation + label));
    } else {
        Q_ASSERT(0 <= label && label < 10);
        Node incidentNode = occupiedNodeIncidentToLabel(label);
        return incidentNode.nodeInDir(labelToDir(label));
    }
}

int Particle::labelToDir(const int label) const
{
    Q_ASSERT(-1 <= tailDir && tailDir < 6);
    if(tailDir == -1) {
        Q_ASSERT(0 <= label && label < 6);
        return (label + orientation) % 6;
    } else {
        Q_ASSERT(0 <= label && label < 10);
        int relativeTailDir = (tailDir - orientation + 6) % 6;
        return (labelDir[relativeTailDir][label] + orientation) % 6;
    }
}

int Particle::labelOfNeighboringNodeInDir(const Node node, const int dir) const
{
    int labelLimit = tailDir == -1 ? 6 : 10;
    for(int label = 0; label < labelLimit; label++) {
        if(labelToDir(label) == dir && neighboringNodeReachedViaLabel(label) == node) {
            return label;
        }
    }
    Q_ASSERT(false);
    return -1; // avoid compiler warning
}

int Particle::headMarkColor() const
{
    return algorithm->headMarkColor;
}

int Particle::headMarkDir() const
{
    Q_ASSERT(-1 <= algorithm->headMarkDir && algorithm->headMarkDir <= 5);
    if(algorithm->headMarkDir == -1) {
        return -1;
    } else {
        return (orientation + algorithm->headMarkDir) % 6;
    }
}

int Particle::tailMarkColor() const
{
    return algorithm->tailMarkColor;
}

int Particle::tailMarkDir() const
{
    Q_ASSERT(-1 <= algorithm->tailMarkDir && algorithm->tailMarkDir <= 5);
    if(algorithm->tailMarkDir == -1) {
        return -1;
    } else {
        return (orientation + algorithm->tailMarkDir) % 6;
    }
}

bool Particle::algorithmIsDeterministic() const
{
    return algorithm->isDeterministic();
}
