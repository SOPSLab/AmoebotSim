#include "labellednocompassparticle.h"

const std::vector<int> LabelledNoCompassParticle::sixLabels = {{0, 1, 2, 3, 4, 5}};
const std::array<const std::vector<int>, 6> LabelledNoCompassParticle::labels =
{{
    {3, 4, 5, 6, 7},
    {4, 5, 6, 7, 8},
    {7, 8, 9, 0, 1},
    {8, 9, 0, 1, 2},
    {9, 0, 1, 2, 3},
    {2, 3, 4, 5, 6}
}};
const std::array<int, 6> LabelledNoCompassParticle::contractLabels = {{0, 1, 4, 5, 6, 9}};
const std::array<std::array<int, 10>, 6> LabelledNoCompassParticle::labelDir =
{{
    {{0, 1, 2, 1, 2, 3, 4, 5, 4, 5}},
    {{0, 1, 2, 3, 2, 3, 4, 5, 0, 5}},
    {{0, 1, 0, 1, 2, 3, 4, 3, 4, 5}},
    {{0, 1, 2, 1, 2, 3, 4, 5, 4, 5}},
    {{0, 1, 2, 3, 2, 3, 4, 5, 0, 5}},
    {{0, 1, 0, 1, 2, 3, 4, 3, 4, 5}}
}};

LabelledNoCompassParticle::LabelledNoCompassParticle(const Node head,
                                   const int tailDir,
                                   const int _orientation)
    : Particle(head, tailDir)
    , orientation(_orientation)
{

}

LabelledNoCompassParticle::~LabelledNoCompassParticle()
{

}

const std::vector<int>& LabelledNoCompassParticle::headLabels() const
{
    Q_ASSERT(-1 <= tailDir && tailDir < 6);
    if(tailDir == -1) {
        return sixLabels;
    } else {
        int relativeTailDir = (tailDir - orientation + 6) % 6;
        return labels[relativeTailDir];
    }
}

const std::vector<int>& LabelledNoCompassParticle::tailLabels() const
{
    Q_ASSERT(-1 <= tailDir && tailDir < 6);
    if(tailDir == -1) {
        return sixLabels;
    } else {
        int relativeTailDir = (tailDir - orientation + 6) % 6;
        return labels[(relativeTailDir + 3) % 6];
    }
}

bool LabelledNoCompassParticle::isHeadLabel(const int label) const
{
    for(auto it = headLabels().cbegin(); it != headLabels().cend(); ++it) {
        if(label == *it) {
            return true;
        }
    }
    return false;
}

bool LabelledNoCompassParticle::isTailLabel(const int label) const
{
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
        if(label == *it) {
            return true;
        }
    }
    return false;
}

int LabelledNoCompassParticle::headContractionLabel() const
{
    Q_ASSERT(0 <= tailDir && tailDir <= 5);
    int relativeTailDir = (tailDir - orientation + 6) % 6;
    return contractLabels[relativeTailDir];
}

int LabelledNoCompassParticle::tailContractionLabel() const
{
    Q_ASSERT(0 <= tailDir && tailDir <= 5);
    int relativeTailDir = (tailDir - orientation + 6) % 6;
    return contractLabels[(relativeTailDir + 3) % 6];
}

Node LabelledNoCompassParticle::occupiedNodeIncidentToLabel(const int label) const
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

Node LabelledNoCompassParticle::neighboringNodeReachedViaLabel(const int label) const
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

int LabelledNoCompassParticle::labelToDir(const int label) const
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

int LabelledNoCompassParticle::labelOfNeighboringNodeInDir(const Node node, const int dir) const
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
