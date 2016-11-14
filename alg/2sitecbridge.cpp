#include <algorithm> // used for find()
#include <math.h> // used for random number generation
#include <QtGlobal>
#include <set>

#include "alg/2sitecbridge.h"
#include "alg/labellednocompassparticle.h"

TwoSiteCBridgeParticle::TwoSiteCBridgeParticle(const Node head,
                                               const int globalTailDir,
                                               const int orientation,
                                               AmoebotSystem& system,
                                               Role role,
                                               const float lambda)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      role(role),
      lambda(lambda)
{
    q = 0; numNbrsBefore = 0; flag = false;
}

void TwoSiteCBridgeParticle::activate()
{
    if(isContracted()) {
        int expandDir = randDir(); // select a random neighboring location
        q = randFloat(0,1); // select a random q in (0,1)

        if(canExpand(expandDir) && !hasExpandedNeighbor()) {
            // count neighbors before expansion
            numNbrsBefore = neighborCount(uniqueLabels());

            expand(expandDir); // expand to the unoccupied position

            flag = !hasExpandedNeighbor();
        }
    }
    else { // is expanded
        if(flag && numNbrsBefore != 5) { // can only attempt to contract to new location if flag == TRUE and original position does not have 5 neighbors
            // count neighbors in new location
            int numNbrsAfter = neighborCount(occupiedLabelsNoExpandedHeads(headLabels()));

            // compute the number of particles that will be in set S after expansion
            int sizeS = 0;
            if(hasNeighborAtLabel(headLabels()[0]) && !(neighborAtLabel(headLabels()[0]).isExpanded() && neighborAtLabel(headLabels()[0]).pointsAtMyHead(*this, headLabels()[0]))) {
                ++sizeS;
            }
            if(hasNeighborAtLabel(headLabels()[4]) && !(neighborAtLabel(headLabels()[4]).isExpanded() && neighborAtLabel(headLabels()[4]).pointsAtMyHead(*this, headLabels()[4]))) {
                ++sizeS;
            }

            // check if the bias probability is satisfied and the locations satisfy property 1 or 2
            if((q < pow(lambda, numNbrsAfter - numNbrsBefore)) && (checkProp1(sizeS) || checkProp2(sizeS))) {
                contractTail(); // contract to new location
            }
            else {
                contractHead(); // contract back to original location
            }
        }
        else {
            contractHead(); // contract back to original location
        }
    }

    return;
}

int TwoSiteCBridgeParticle::headMarkColor() const
{
    switch(role) {
    case Role::Object: return 0x000000;
    case Role::Particle: return -1;
    }

    return -1;
}

QString TwoSiteCBridgeParticle::inspectionText() const
{
    QString text;
    text = "Properties:\n";
    text += "    lambda = " + QString::number(lambda) + ",\n";
    text += "    q in (0,1) = " + QString::number(q) + ",\n";
    text += "    flag = " + QString::number(flag) + ".\n";

    if(isContracted()) {
        text += "Contracted properties:\n";
        text += "    #neighbors in first position = " + QString::number(numNbrsBefore) + ",\n";
    }
    else { // is expanded
        text += "Expanded properties:\n";
        text += "    #neighbors in first position = " + QString::number(numNbrsBefore) + ",\n";
        text += "    #neighbors in second position = " + QString::number(neighborCount(occupiedLabelsNoExpandedHeads(headLabels()))) + ",\n";
    }

    return text;
}

TwoSiteCBridgeParticle& TwoSiteCBridgeParticle::neighborAtLabel(int label) const
{
    return AmoebotParticle::neighborAtLabel<TwoSiteCBridgeParticle>(label);
}

bool TwoSiteCBridgeParticle::hasExpandedNeighbor() const
{
    for(const int label: uniqueLabels()) {
        if(hasNeighborAtLabel(label) && neighborAtLabel(label).isExpanded()) {
            return true;
        }
    }

    return false;
}

int TwoSiteCBridgeParticle::neighborCount(std::vector<int> labels) const
{
    int neighbors = 0;

    for(const int label : labels) {
        if(hasNeighborAtLabel(label)) {
            ++neighbors;
        }
    }

    return neighbors;
}

bool TwoSiteCBridgeParticle::checkProp1(const int sizeS) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(0 <= sizeS && sizeS <= 2);
    Q_ASSERT(flag); // not required by algorithm, but equivalent and cleaner for our implementation

    if(sizeS != 0) { // S has to be nonempty for Property 1
        const std::vector<int> allLabels = uniqueLabels();
        const std::vector<int> occLabels = occupiedLabelsNoExpandedHeads(allLabels);

        // find the first position which is occupied but the previous (clockwise) is not
        int firstOccupiedIndex = -1;
        for(int i = 0; (size_t)i < allLabels.size(); ++i) {
            if(std::find(occLabels.begin(), occLabels.end(), allLabels[i]) != occLabels.end() &&
               std::find(occLabels.begin(), occLabels.end(), allLabels[(i + allLabels.size() - 1) % allLabels.size()]) == occLabels.end()) {
                firstOccupiedIndex = i;
                break;
            }
        }
        Q_ASSERT(firstOccupiedIndex != -1); // DEBUG: sanity check

        // proceeding counter-clockwise, count the number of consecutive occupied positions
        int consecutiveCount = 0;
        for(int offset = 0; (size_t)offset < allLabels.size(); ++offset) {
            if(std::find(occLabels.begin(), occLabels.end(), allLabels[(firstOccupiedIndex + offset) % allLabels.size()]) != occLabels.end()) {
                ++consecutiveCount;
            }
            else {
                break;
            }
        }

        return (size_t)consecutiveCount == occLabels.size();
    }
    else {
        return false;
    }
}

bool TwoSiteCBridgeParticle::checkProp2(const int sizeS) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(0 <= sizeS && sizeS <= 2);
    Q_ASSERT(flag); // not required by algorithm, but equivalent and cleaner for our implementation

    const std::vector<int> occLabels = occupiedLabelsNoExpandedHeads(uniqueLabels());

    if(sizeS == 0) { // S has to be empty for Property 2
        // both nodes occupied by the particle need to have at least one neighbor
        bool hasHeadNeighbor = false, hasTailNeighbor = false;
        for(int i = 1; i <= 3; ++i) { // only need the head labels not in S
            if(std::find(occLabels.begin(), occLabels.end(), headLabels()[i]) != occLabels.end()) {
                hasHeadNeighbor = true;
                break;
            }
        }
        for(int i = 1; i <= 3; ++i) { // only need the tail labels not in S
            if(std::find(occLabels.begin(), occLabels.end(), tailLabels()[i]) != occLabels.end()) {
                hasTailNeighbor = true;
                break;
            }
        }

        // all neighbors of the head must be consecutive
        bool hasConsecutiveHead = false;
        if(hasHeadNeighbor) {
            int firstOccupiedHeadIndex = -1;
            for(int i = 1; i <= 3; ++i) { // again, we only care about the head labels not in S
                if(std::find(occLabels.begin(), occLabels.end(), headLabels()[i]) != occLabels.end()) {
                    firstOccupiedHeadIndex = i;
                    break;
                }
            }
            Q_ASSERT(firstOccupiedHeadIndex != -1); // DEBUG: sanity check

            int consecutiveHeadCount = 0;
            for(int i = firstOccupiedHeadIndex; i <= 3; ++i) { // iterate from the first occupied head index to the last head label before S (headLabel[3])
                if(std::find(occLabels.begin(), occLabels.end(), headLabels()[i]) != occLabels.end()) {
                    ++consecutiveHeadCount;
                }
                else {
                    break;
                }
            }

            // count number of head labels (not in S) in occLabels
            int totalHeadNeighbors = 0;
            for(int i = 1; i <= 3; ++i) {
                if(std::find(occLabels.begin(), occLabels.end(), headLabels()[i]) != occLabels.end()) {
                    ++totalHeadNeighbors;
                }
            }

            hasConsecutiveHead = consecutiveHeadCount == totalHeadNeighbors;
        }

        // all neighbors of the tail must be consecutive
        bool hasConsecutiveTail = false;
        if(hasTailNeighbor) {
            int firstOccupiedTailIndex = -1;
            for(int i = 1; i <= 3; ++i) { // again, only need tail labels not in S
                if(std::find(occLabels.begin(), occLabels.end(), tailLabels()[i]) != occLabels.end()) {
                    firstOccupiedTailIndex = i;
                    break;
                }
            }
            Q_ASSERT(firstOccupiedTailIndex != -1); // DEBUG: sanity check

            int consecutiveTailCount = 0;
            for(int i = firstOccupiedTailIndex; i <= 3; ++i) { // iterate from first occupied tail label to last tail label before S (tailLabel[3])
                if(std::find(occLabels.begin(), occLabels.end(), tailLabels()[i]) != occLabels.end()) {
                    ++consecutiveTailCount;
                }
                else {
                    break;
                }
            }

            // count number of tail labels (not in S) in occLabels
            int totalTailNeighbors = 0;
            for(int i = 1; i <= 3; ++i) {
                if(std::find(occLabels.begin(), occLabels.end(), tailLabels()[i]) != occLabels.end()) {
                    ++totalTailNeighbors;
                }
            }

            hasConsecutiveTail = consecutiveTailCount == totalTailNeighbors;
        }

        return hasHeadNeighbor && hasTailNeighbor && hasConsecutiveHead && hasConsecutiveTail;
    }
    else {
        return false;
    }
}

const std::vector<int> TwoSiteCBridgeParticle::uniqueLabels() const
{
    if(isContracted()) {
        return {0, 1, 2, 3, 4, 5};
    }
    else { // is expanded
        std::vector<int> labels;
        for(int label = 0; label < 10; ++label) {
            if(neighboringNodeReachedViaLabel(label) != neighboringNodeReachedViaLabel((label + 9) % 10)) {
                labels.push_back(label);
            }
        }

        Q_ASSERT(labels.size() == 8); // sanity check
        return labels;
    }
}

const std::vector<int> TwoSiteCBridgeParticle::occupiedLabelsNoExpandedHeads(std::vector<int> labels) const
{
    Q_ASSERT(flag); // expanded particles with flag == TRUE can ignore heads of expanded neighbors since their flag == FALSE

    std::vector<int> occNoExpHeadLabels;
    for(const int label : labels) {
        // if the label points at the head of an expanded neighboring particle, do not include it
        if(hasNeighborAtLabel(label) && !(neighborAtLabel(label).isExpanded() && neighborAtLabel(label).pointsAtMyHead(*this, label)))
        {
            occNoExpHeadLabels.push_back(label);
        }
    }

    return occNoExpHeadLabels;
}

// TODO: update initialization
TwoSiteCBridgeSystem::TwoSiteCBridgeSystem(int numParticles, float lambda)
{
    Q_ASSERT(lambda > 1);

    // generate a straight line of particles
    for(int i = 0; i < numParticles; ++i) {
        insert(new TwoSiteCBridgeParticle(Node(i, 0), -1, randDir(), *this, TwoSiteCBridgeParticle::Role::Particle, lambda));
    }
}

bool TwoSiteCBridgeSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    // terminates on disconnection
    if(!isConnected(particles)) {
        return true;
    }
#endif

    return false;
}

