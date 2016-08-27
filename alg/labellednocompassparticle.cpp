#include "alg/labellednocompassparticle.h"

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

LabelledNoCompassParticle::LabelledNoCompassParticle(const Node& head, int globalTailDir, int orientation)
    : Particle(head, globalTailDir), orientation(orientation)
{
    Q_ASSERT(0 <= orientation && orientation < 6);
}

// returns the local direction of the tail relative to the head
int LabelledNoCompassParticle::tailDir() const
{
    Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
    if(isContracted()) {
        return -1;
    } else {
        return globalToLocalDir(globalTailDir);
    }
}

// returns the local direction the edge with label 'label' points to
int LabelledNoCompassParticle::labelToDir(int label) const
{
    Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
    if(isContracted()) {
        Q_ASSERT(0 <= label && label < 6);
        return label;
    } else {
        Q_ASSERT(0 <= label && label < 10);
        return labelDir[tailDir()][label];
    }
}

// returns the local direction the edge with label 'label' would point to after an expansion
// in the local direction 'expansionDir'
int LabelledNoCompassParticle::labelToDirAfterExpansion(int label, int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= label && label < 10);
    Q_ASSERT(0 <= expansionDir && expansionDir < 6);
    return labelDir[(expansionDir + 3) % 6][label];
}

// returns an array of labels of edges incident to the particle's head
const std::vector<int>& LabelledNoCompassParticle::headLabels() const
{
    Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
    if(isContracted()) {
        return sixLabels;
    } else {
        return labels[tailDir()];
    }
}

// returns an array of labels of edges incident to the particle's tail
const std::vector<int>& LabelledNoCompassParticle::tailLabels() const
{
    Q_ASSERT(isExpanded());
    return labels[(tailDir() + 3) % 6];
}

// returns true if 'label' is a head label, false otherwise
bool LabelledNoCompassParticle::isHeadLabel(int label) const
{
    Q_ASSERT(0 <= label && label < 10);
    for(const int headLabel : headLabels()) {
        if(label == headLabel) {
            return true;
        }
    }
    return false;
}

// returns true if 'label' is a tail label, false otherwise
bool LabelledNoCompassParticle::isTailLabel(int label) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(0 <= label && label < 10);
    for(const int tailLabel : tailLabels()) {
        if(label == tailLabel) {
            return true;
        }
    }
    return false;
}

// returns the head label pointing in local direction 'dir'
// NOTE: in the case of an expanded particle with its tail in the local direction d
//       from its head, dirToHeadLabel(d) would cause an error because the edge from
//       head to tail does not have a label
int LabelledNoCompassParticle::dirToHeadLabel(int dir) const
{
    Q_ASSERT(0 <= dir && dir < 6);
    for(const int headLabel : headLabels()) {
        if(dir == labelToDir(headLabel)) {
            return headLabel;
        }
    }
    Q_ASSERT(false);
    return 0; // avoid compiler warning
}

// returns the tail label pointing in the local direction 'dir'
// NOTE: analagous to above, if an expanded particle has its head in the local
//       direction d from its tail, dirToTailLabel(d) would cause an error because
//       the edge from tail to head does not have a label
int LabelledNoCompassParticle::dirToTailLabel(int dir) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(0 <= dir && dir < 6);
    for(const int tailLabel : tailLabels()) {
        if(dir == labelToDir(tailLabel)) {
            return tailLabel;
        }
    }
    Q_ASSERT(false);
    return 0; // avoid compiler warning
}

// returns label needed for AmoebotParticle::contract() to execute a head contraction
int LabelledNoCompassParticle::headContractionLabel() const
{
    Q_ASSERT(isExpanded());
    return contractLabels[tailDir()];
}

// returns label needed for AmoebotParticle::contract() to execute a tail contraction
int LabelledNoCompassParticle::tailContractionLabel() const
{
    Q_ASSERT(isExpanded());
    return contractLabels[(tailDir() + 3) % 6];
}

/*
 * The ____AfterExpansion(int expansionDir) functions return exactly what their corresponding
 * non-after-expansion versions would if the particle were to first expand in the local direction
 * 'expansionDir'
 */

const std::vector<int>& LabelledNoCompassParticle::headLabelsAfterExpansion(int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir < 6);
    const int tempTailDir = (expansionDir + 3) % 6;
    return labels[tempTailDir];
}

const std::vector<int>& LabelledNoCompassParticle::tailLabelsAfterExpansion(int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir < 6);
    const int tempTailDir = (expansionDir + 3) % 6;
    return labels[(tempTailDir + 3) % 6];
}

bool LabelledNoCompassParticle::isHeadLabelAfterExpansion(int label, int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir < 6);
    for(const int headLabel : headLabelsAfterExpansion(expansionDir)) {
        if(label == headLabel) {
            return true;
        }
    }
    return false;
}

bool LabelledNoCompassParticle::isTailLabelAfterExpansion(int label, int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir < 6);
    for(const int tailLabel : tailLabelsAfterExpansion(expansionDir)) {
        if(label == tailLabel) {
            return true;
        }
    }
    return false;
}

int LabelledNoCompassParticle::dirToHeadLabelAfterExpansion(int dir, int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= dir && dir < 6);
    Q_ASSERT(0 <= expansionDir && expansionDir < 6);
    for(const int headLabel : headLabelsAfterExpansion(expansionDir)) {
        if(dir == labelToDirAfterExpansion(headLabel, expansionDir)) {
            return headLabel;
        }
    }
    Q_ASSERT(false);
    return 0; // avoid compiler warning
}

int LabelledNoCompassParticle::dirToTailLabelAfterExpansion(int dir, int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= dir && dir < 6);
    Q_ASSERT(0 <= expansionDir && expansionDir < 6);
    for(const int tailLabel : tailLabelsAfterExpansion(expansionDir)) {
        if(dir == labelToDirAfterExpansion(tailLabel, expansionDir)) {
            return tailLabel;
        }
    }
    Q_ASSERT(false);
    return 0; // avoid compiler warning
}

int LabelledNoCompassParticle::headContractionLabelAfterExpansion(int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir < 6);
    return contractLabels[(expansionDir + 3) % 6];
}

int LabelledNoCompassParticle::tailContractionLabelAfterExpansion(int expansionDir) const
{
    Q_ASSERT(isContracted());
    Q_ASSERT(0 <= expansionDir && expansionDir < 6);
    return contractLabels[expansionDir];
}

// returns the global direction the edge with label 'label' points to
int LabelledNoCompassParticle::labelToGlobalDir(int label) const
{
    Q_ASSERT(0 <= label && label < 10);
    return localToGlobalDir(labelToDir(label));
}

// returns the label of the edge that connects the particle to node 'node'
int LabelledNoCompassParticle::labelOfNeighboringNodeInGlobalDir(const Node& node, int globalDir) const
{
    Q_ASSERT(0 <= globalDir && globalDir < 6);
    const int labelLimit = (isContracted()) ? 6 : 10;
    for(int label = 0; label < labelLimit; label++) {
        if(labelToGlobalDir(label) == globalDir && neighboringNodeReachedViaLabel(label) == node) {
            return label;
        }
    }
    Q_ASSERT(false);
    return 0; // avoid compiler warning
}

// returns the head node if 'label' is a head label and the tail node otherwise
Node LabelledNoCompassParticle::occupiedNodeIncidentToLabel(int label) const
{
    Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
    if(isContracted()) {
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

// returns the node reached by the edge with label 'label'
Node LabelledNoCompassParticle::neighboringNodeReachedViaLabel(int label) const
{
    Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
    if(isContracted()) {
        Q_ASSERT(0 <= label && label < 6);
        const int dir = (orientation + label) % 6;
        return head.nodeInDir(dir);
    } else {
        Q_ASSERT(0 <= label && label < 10);
        Node incidentNode = occupiedNodeIncidentToLabel(label);
        return incidentNode.nodeInDir(labelToGlobalDir(label));
    }
}

// returns the global direction associated with the local direction 'dir'
int LabelledNoCompassParticle::localToGlobalDir(int dir) const
{
    Q_ASSERT(0 <= dir && dir < 6);
    return (orientation + dir) % 6;
}

// returns the local direction associated with the global direction 'globalDir'
int LabelledNoCompassParticle::globalToLocalDir(int globalDir) const
{
    Q_ASSERT(0 <= globalDir && globalDir < 6);
    return (globalDir - orientation + 6) % 6;
}

// returns the equivalent local direction from this particle's perspective of the local direction 'neighborDir'
// from 'neighbor' particle's perspective
int LabelledNoCompassParticle::neighborDirToDir(const LabelledNoCompassParticle& neighbor, int neighborDir) const
{
    Q_ASSERT(0 <= neighborDir && neighborDir < 6);
    return globalToLocalDir(neighbor.localToGlobalDir(neighborDir));
}

// returns the equivalent local direction from 'neighbor' particle's perspective of the local direction 'myDir'
// from this particle's perspective
int LabelledNoCompassParticle::dirToNeighborDir(const LabelledNoCompassParticle& neighbor, int myDir) const
{
    Q_ASSERT(0 <= myDir && myDir < 6);
    return neighbor.globalToLocalDir(localToGlobalDir(myDir));
}

// returns true if the 'neighbor' particle's edge with label 'neighborLabel' is incident to a node occupied by
// this particle, false otherwise
bool LabelledNoCompassParticle::pointsAtMe(const LabelledNoCompassParticle& neighbor, int neighborLabel) const
{
    Q_ASSERT(0 <= neighborLabel && neighborLabel < 10);
    if(isContracted()) {
        return pointsAtMyHead(neighbor, neighborLabel);
    } else {
        return pointsAtMyHead(neighbor, neighborLabel) || pointsAtMyTail(neighbor, neighborLabel);
    }
}

// returns true if the 'neighbor' particle's edge with label 'neighborLabel' is incident to the head node of this
// particle, false otherwise
bool LabelledNoCompassParticle::pointsAtMyHead(const LabelledNoCompassParticle& neighbor, int neighborLabel) const
{
    Q_ASSERT(0 <= neighborLabel && neighborLabel < 10);
    return neighbor.neighboringNodeReachedViaLabel(neighborLabel) == head;
}

// returns true if the 'neighbor' particle's edge with label 'neighborLabel' is incident to the tail node of this
// particle, false otherwise
bool LabelledNoCompassParticle::pointsAtMyTail(const LabelledNoCompassParticle& neighbor, int neighborLabel) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(0 <= neighborLabel && neighborLabel < 10);
    return neighbor.neighboringNodeReachedViaLabel(neighborLabel) == tail();
}
