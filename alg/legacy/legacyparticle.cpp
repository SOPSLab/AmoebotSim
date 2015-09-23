#include "legacyparticle.h"

const std::vector<int> LegacyParticle::sixLabels = {{0, 1, 2, 3, 4, 5}};
const std::array<const std::vector<int>, 6> LegacyParticle::labels =
{{
    {3, 4, 5, 6, 7},
    {4, 5, 6, 7, 8},
    {7, 8, 9, 0, 1},
    {8, 9, 0, 1, 2},
    {9, 0, 1, 2, 3},
    {2, 3, 4, 5, 6}
}};
const std::array<int, 6> LegacyParticle::contractLabels = {{0, 1, 4, 5, 6, 9}};
const std::array<std::array<int, 10>, 6> LegacyParticle::labelDir =
{{
    {{0, 1, 2, 1, 2, 3, 4, 5, 4, 5}},
    {{0, 1, 2, 3, 2, 3, 4, 5, 0, 5}},
    {{0, 1, 0, 1, 2, 3, 4, 3, 4, 5}},
    {{0, 1, 2, 1, 2, 3, 4, 5, 4, 5}},
    {{0, 1, 2, 3, 2, 3, 4, 5, 0, 5}},
    {{0, 1, 0, 1, 2, 3, 4, 3, 4, 5}}
}};

LegacyParticle::LegacyParticle(std::shared_ptr<Algorithm> _algorithm,
                               const int _orientation,
                               const Node head,
                               const int tailDir) :
    Particle(head, tailDir),
    orientation(_orientation),
    algorithm(_algorithm)
{
    Q_ASSERT(_algorithm != nullptr);
}

LegacyParticle::~LegacyParticle()
{

}

Movement LegacyParticle::executeAlgorithm(std::array<const Flag*, 10>& inFlag)
{
    Q_ASSERT(newAlgorithm == nullptr);
    Q_ASSERT(!algorithm->isStatic());
    newAlgorithm = algorithm->clone();
    return newAlgorithm->execute(inFlag);
}

void LegacyParticle::apply()
{
    Q_ASSERT(newAlgorithm != nullptr);
    algorithm = newAlgorithm;
    newAlgorithm.reset();
}

void LegacyParticle::discard()
{
    Q_ASSERT(newAlgorithm != nullptr);
    newAlgorithm.reset();
}

const Flag* LegacyParticle::getFlagForNodeInDir(const Node node, const int dir)
{
    return algorithm->flagAt(labelOfNeighboringNodeInDir(node, dir));
}

std::shared_ptr<const Algorithm> LegacyParticle::getAlgorithm() const{
  return std::const_pointer_cast<const Algorithm>(algorithm);
}

bool LegacyParticle::algorithmIsDeterministic() const
{
    return algorithm->isDeterministic();
}

bool LegacyParticle::isStatic() const
{
    return algorithm->isStatic();
}

const std::vector<int>& LegacyParticle::headLabels() const
{
    Q_ASSERT(-1 <= tailDir && tailDir < 6);
    if(tailDir == -1) {
        return sixLabels;
    } else {
        int relativeTailDir = (tailDir - orientation + 6) % 6;
        return labels[relativeTailDir];
    }
}

const std::vector<int>& LegacyParticle::tailLabels() const
{
    Q_ASSERT(-1 <= tailDir && tailDir < 6);
    if(tailDir == -1) {
        return sixLabels;
    } else {
        int relativeTailDir = (tailDir - orientation + 6) % 6;
        return labels[(relativeTailDir + 3) % 6];
    }
}

bool LegacyParticle::isHeadLabel(const int label) const
{
    for(auto it = headLabels().cbegin(); it != headLabels().cend(); ++it) {
        if(label == *it) {
            return true;
        }
    }
    return false;
}

bool LegacyParticle::isTailLabel(const int label) const
{
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        if(label == *it) {
            return true;
        }
    }
    return false;
}

int LegacyParticle::headContractionLabel() const
{
    Q_ASSERT(0 <= tailDir && tailDir <= 5);
    int relativeTailDir = (tailDir - orientation + 6) % 6;
    return contractLabels[relativeTailDir];
}

int LegacyParticle::tailContractionLabel() const
{
    Q_ASSERT(0 <= tailDir && tailDir <= 5);
    int relativeTailDir = (tailDir - orientation + 6) % 6;
    return contractLabels[(relativeTailDir + 3) % 6];
}

Node LegacyParticle::occupiedNodeIncidentToLabel(const int label) const
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

Node LegacyParticle::neighboringNodeReachedViaLabel(const int label) const
{
    Q_ASSERT(-1 <= tailDir && tailDir < 6);
    if(tailDir == -1) {
        Q_ASSERT(0 <= label && label < 6);
        int dir = orientation + label;
        dir = (dir % 6 + 6) % 6;
        return head.nodeInDir(dir);
    } else {
        Q_ASSERT(0 <= label && label < 10);
        Node incidentNode = occupiedNodeIncidentToLabel(label);
        return incidentNode.nodeInDir(labelToDir(label));
    }
}

int LegacyParticle::labelToDir(const int label) const
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

int LegacyParticle::labelOfNeighboringNodeInDir(const Node node, const int dir) const
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

int LegacyParticle::headMarkColor() const
{
    return algorithm->headMarkColor;
}

int LegacyParticle::headMarkDir() const
{
    Q_ASSERT(-1 <= algorithm->headMarkDir && algorithm->headMarkDir <= 5);
    if(algorithm->headMarkDir == -1) {
        return -1;
    } else {
        return (orientation + algorithm->headMarkDir) % 6;
    }
}

int LegacyParticle::tailMarkColor() const
{
    return algorithm->tailMarkColor;
}

int LegacyParticle::tailMarkDir() const
{
    Q_ASSERT(-1 <= algorithm->tailMarkDir && algorithm->tailMarkDir <= 5);
    if(algorithm->tailMarkDir == -1) {
        return -1;
    } else {
        return (orientation + algorithm->tailMarkDir) % 6;
    }
}

std::array<int, 18> LegacyParticle::borderColors() const
{
    std::array<int, 18> rearranged;

    for(unsigned int i = 0; i < 18; i++) {
        rearranged[borderDir(i)] = algorithm->borderColors[i];
    }

    return rearranged;
}

int LegacyParticle::borderDir(const int dir) const
{
    Q_ASSERT(-1 <= dir && dir <= 17);
    if(dir == -1) {
        return -1;
    } else {
        return (3 * orientation + dir) % 18;
    }
}

std::array<int, 6> LegacyParticle::borderPointColors() const
{
    std::array<int, 6> rearranged;

    for(unsigned int i = 0; i < 6; i++) {
        rearranged[borderPointDir(i)] = algorithm->borderPointColors[i];
    }

    return rearranged;
}

int LegacyParticle::borderPointDir(const int dir) const
{
    Q_ASSERT(-1 <= dir && dir <= 5);
    if(dir == -1) {
        return -1;
    } else {
        return (orientation + dir) % 6;
    }
}
