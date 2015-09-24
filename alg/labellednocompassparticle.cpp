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

int LabelledNoCompassParticle::tailDir() const
{
    return globalToLocalDir(globalTailDir);
}

const std::vector<int>& LabelledNoCompassParticle::headLabels() const
{
    Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
    if(globalTailDir == -1) {
        return sixLabels;
    } else {
        return labels[tailDir()];
    }
}

const std::vector<int>& LabelledNoCompassParticle::tailLabels() const
{
    Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
    if(globalTailDir == -1) {
        return sixLabels;
    } else {
        return labels[(tailDir() + 3) % 6];
    }
}

bool LabelledNoCompassParticle::isHeadLabel(const int label) const
{
    for(int headLabel : headLabels()) {
        if(label == headLabel) {
            return true;
        }
    }
    return false;
}

bool LabelledNoCompassParticle::isTailLabel(const int label) const
{
    for(int tailLabel : tailLabels()) {
        if(label == tailLabel) {
            return true;
        }
    }
    return false;
}

int LabelledNoCompassParticle::dirToHeadLabel(int dir) const
{
    Q_ASSERT(0 <= dir && dir <= 5);
    for(int headLabel : headLabels()) {
        if(dir == labelToDir(headLabel)) {
            return headLabel;
        }
    }
    Q_ASSERT(false);
    return 0; // avoid compiler warning
}

int LabelledNoCompassParticle::dirToTailLabel(int dir) const
{
    Q_ASSERT(0 <= dir && dir <= 5);
    for(int tailLabel : tailLabels()) {
        if(dir == labelToDir(tailLabel)) {
            return tailLabel;
        }
    }
    Q_ASSERT(false);
    return 0; // avoid compiler warning
}

int LabelledNoCompassParticle::headContractionLabel() const
{
    Q_ASSERT(0 <= globalTailDir && globalTailDir <= 5);
    return contractLabels[tailDir()];
}

int LabelledNoCompassParticle::tailContractionLabel() const
{
    Q_ASSERT(0 <= globalTailDir && globalTailDir <= 5);
    return contractLabels[(tailDir() + 3) % 6];
}

const std::vector<int>& LabelledNoCompassParticle::headLabelsAfterExpansion(const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    int tempTailDir = (expansionDir + 3) % 6;
    return labels[tempTailDir];
}

const std::vector<int>& LabelledNoCompassParticle::tailLabelsAfterExpansion(const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    int tempTailDir = (expansionDir + 3) % 6;
    return labels[(tempTailDir + 3) % 6];
}

bool LabelledNoCompassParticle::isHeadLabelAfterExpansion(const int label, const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    for(int headLabel : headLabelsAfterExpansion(expansionDir)) {
        if(label == headLabel) {
            return true;
        }
    }
    return false;
}

bool LabelledNoCompassParticle::isTailLabelAfterExpansion(const int label, const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    for(int tailLabel : tailLabelsAfterExpansion(expansionDir)) {
        if(label == tailLabel) {
            return true;
        }
    }
    return false;
}

int LabelledNoCompassParticle::dirToHeadLabelAfterExpansion(const int dir, const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= dir && dir <= 5);
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    for(int headLabel : headLabelsAfterExpansion(expansionDir)) {
        if(dir == labelToDirAfterExpansion(headLabel, expansionDir)) {
            return headLabel;
        }
    }
    Q_ASSERT(false);
    return 0; // avoid compiler warning
}

int LabelledNoCompassParticle::dirToTailLabelAfterExpansion(const int dir, const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= dir && dir <= 5);
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    for(int tailLabel : tailLabelsAfterExpansion(expansionDir)) {
        if(dir == labelToDirAfterExpansion(tailLabel, expansionDir)) {
            return tailLabel;
        }
    }
    Q_ASSERT(false);
    return 0; // avoid compiler warning
}

int LabelledNoCompassParticle::headContractionLabelAfterExpansion(const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    return contractLabels[(expansionDir + 3) % 6];
}

int LabelledNoCompassParticle::tailContractionLabelAfterExpansion(const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir <= 5);
    return contractLabels[expansionDir];
}

Node LabelledNoCompassParticle::occupiedNodeIncidentToLabel(const int label) const
{
    Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
    if(globalTailDir == -1) {
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
    Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
    if(globalTailDir == -1) {
        Q_ASSERT(0 <= label && label < 6);
        int dir = orientation + label;
        dir = (dir % 6 + 6) % 6;
        return head.nodeInDir(dir);
    } else {
        Q_ASSERT(0 <= label && label < 10);
        Node incidentNode = occupiedNodeIncidentToLabel(label);
        return incidentNode.nodeInDir(labelToGlobalDir(label));
    }
}

int LabelledNoCompassParticle::labelToDir(const int label) const
{
    Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
    if(globalTailDir == -1) {
        Q_ASSERT(0 <= label && label < 6);
        return label;
    } else {
        Q_ASSERT(0 <= label && label < 10);
        return labelDir[tailDir()][label];
    }
}

int LabelledNoCompassParticle::labelToDirAfterExpansion(const int label, const int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir < 6);
    return labelDir[(expansionDir + 3) % 6][label];
}

int LabelledNoCompassParticle::labelToGlobalDir(const int label) const
{
    return localToGlobalDir(labelToDir(label));
}

int LabelledNoCompassParticle::labelOfNeighboringNodeInGlobalDir(const Node node, const int globalDir) const
{
    int labelLimit = globalTailDir == -1 ? 6 : 10;
    for(int label = 0; label < labelLimit; label++) {
        if(labelToGlobalDir(label) == globalDir && neighboringNodeReachedViaLabel(label) == node) {
            return label;
        }
    }
    Q_ASSERT(false);
    return 0; // avoid compiler warning
}

int LabelledNoCompassParticle::localToGlobalDir(int dir) const
{
    Q_ASSERT(0 <= dir && dir <= 5);
    return (orientation + dir) % 6;
}

int LabelledNoCompassParticle::globalToLocalDir(int globalDir) const
{
    Q_ASSERT(0 <= globalDir && globalDir <= 5);
    return (globalDir - orientation + 6) % 6;
}

int LabelledNoCompassParticle::neighborDirToDir(const LabelledNoCompassParticle& neighbor, int neighborDir) const
{
    return globalToLocalDir(neighbor.localToGlobalDir(neighborDir));
}

int LabelledNoCompassParticle::dirToNeighborDir(const LabelledNoCompassParticle& neighbor, int myDir) const
{
    return neighbor.globalToLocalDir(localToGlobalDir(myDir));
}

bool LabelledNoCompassParticle::pointsAtMe(const LabelledNoCompassParticle& neighbor, int neighborLabel) const
{
    if(isContracted()) {
        return pointsAtMyHead(neighbor, neighborLabel);
    } else {
        return pointsAtMyHead(neighbor, neighborLabel) || pointsAtMyTail(neighbor, neighborLabel);
    }
}

bool LabelledNoCompassParticle::pointsAtMyHead(const LabelledNoCompassParticle& neighbor, int neighborLabel) const
{
    return neighbor.neighboringNodeReachedViaLabel(neighborLabel) == head;
}

bool LabelledNoCompassParticle::pointsAtMyTail(const LabelledNoCompassParticle& neighbor, int neighborLabel) const
{
    Q_ASSERT(isExpanded());
    return neighbor.neighboringNodeReachedViaLabel(neighborLabel) == tail();
}
