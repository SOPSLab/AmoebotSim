#include <algorithm> // used for find()
#include <math.h> // used for random number generation
#include <QtGlobal>
#include <set>

#include "alg/compression.h"
#include "alg/labellednocompassparticle.h"

CompressionParticle::CompressionParticle(const Node head,
                                         const int globalTailDir,
                                         const int orientation,
                                         AmoebotSystem& system,
                                         const float lambda)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      lambda(lambda)
{
    q = 0; numNeighbors = 0; numTriBefore = 0; flag = false;
}

void CompressionParticle::activate()
{
    if(isContracted()) {

        int expandDir = randDir(); // select a random neighboring location
        q = randFloat(0,1); // select a random q in (0,1)

        if(canExpand(expandDir) && !hasExpandedNeighbor()) {
            // count neighbors and triangles before expansion
            numNeighbors = neighborCount(uniqueLabels());
            numTriBefore = triangleCount();

            expand(expandDir); // expand to the unoccupied position

            flag = !hasExpandedNeighbor();
        }
    }
    else { // is expanded
        if(flag && numNeighbors != 5) { // can only attempt to contract to new location if flag == TRUE and original position does not have 5 neighbors
            // count triangles formed in new location
            int numTriAfter = triangleCount();

            // compute the number of particles that will be in set S after expansion
            int sizeS = 0;
            if(hasNeighborAtLabel(headLabels()[0]) && !(neighborAtLabel(headLabels()[0]).isExpanded() && neighborAtLabel(headLabels()[0]).pointsAtMyHead(*this, headLabels()[0]))) {
                ++sizeS;
            }
            if(hasNeighborAtLabel(headLabels()[4]) && !(neighborAtLabel(headLabels()[4]).isExpanded() && neighborAtLabel(headLabels()[4]).pointsAtMyHead(*this, headLabels()[4]))) {
                ++sizeS;
            }

            // check if the bias probability is satisfied and the locations satisfy property 1 or 2
            if((q < pow(lambda, numTriAfter - numTriBefore)) && (checkProp1(sizeS) || checkProp2(sizeS))) {
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

QString CompressionParticle::inspectionText() const
{
    QString text;
    text = "Properties:\n";
    text += "    lambda = " + QString::number(lambda) + ",\n";
    text += "    q in (0,1) = " + QString::number(q) + ",\n";
    text += "    flag = " + QString::number(flag) + ".\n";

    if(isContracted()) {
        text += "Contracted properties:\n";
        text += "    numNeighbors in first position = " + QString::number(numNeighbors) + ",\n";
        text += "    numTriangles in first position = " + QString::number(numTriBefore) + ".\n";
    }
    else { // is expanded
        text += "Expanded properties:\n";
        text += "    numNeighbors in first position = " + QString::number(numNeighbors) + ",\n";
        text += "    numTriangles in first position = " + QString::number(numTriBefore) + ",\n";
        text += "    numTriangles in second position = " + QString::number(triangleCount()) + ",\n";
    }

    return text;
}

CompressionParticle& CompressionParticle::neighborAtLabel(int label) const
{
    return AmoebotParticle::neighborAtLabel<CompressionParticle>(label);
}

bool CompressionParticle::hasExpandedNeighbor() const
{
    for(const int label: uniqueLabels()) {
        if(hasNeighborAtLabel(label) && neighborAtLabel(label).isExpanded()) {
            return true;
        }
    }

    return false;
}

int CompressionParticle::neighborCount(std::vector<int> labels) const
{
    int neighbors = 0;

    for(const int label : labels) {
        if(hasNeighborAtLabel(label)) {
            ++neighbors;
        }
    }

    return neighbors;
}

int CompressionParticle::triangleCount() const
{
    int triangles = 0;
    int limit = (isContracted()) ? 6 : 4; // contracted could have six triangles, expanded should at most four

    for(int i = 0; i < limit; ++i) {
        if(hasNeighborAtLabel(headLabels()[i]) && hasNeighborAtLabel(headLabels()[(i + 1) % headLabels().size()])) {
            ++triangles;
        }
    }

    return triangles;
}

bool CompressionParticle::checkProp1(const int sizeS) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(0 <= sizeS && sizeS <= 2);
    Q_ASSERT(flag); // not required by algorithm, but equivalent and cleaner for our implementation

    if(sizeS != 0) { // S has to be nonempty for Property 1
        const std::vector<int> allLabels = uniqueLabels();
        const std::vector<int> occLabels = occupiedLabelsNoExpandedHeads();

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

bool CompressionParticle::checkProp2(const int sizeS) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(0 <= sizeS && sizeS <= 2);
    Q_ASSERT(flag); // not required by algorithm, but equivalent and cleaner for our implementation

    const std::vector<int> occLabels = occupiedLabelsNoExpandedHeads();

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

const std::vector<int> CompressionParticle::uniqueLabels() const
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

        Q_ASSERT(labels.size() == 8); // DEBUG: sanity check
        return labels;
    }
}

const std::vector<int> CompressionParticle::occupiedLabelsNoExpandedHeads() const
{
    Q_ASSERT(flag); // expanded particles with flag == TRUE can ignore heads of expanded neighbors since their flag == FALSE

    std::vector<int> labels;
    for(const int label : uniqueLabels()) {
        // if the label points at the head of an expanded neighboring particle, do not include it
        if(hasNeighborAtLabel(label) && !(neighborAtLabel(label).isExpanded() && neighborAtLabel(label).pointsAtMyHead(*this, label)))
        {
            labels.push_back(label);
        }
    }

    return labels;
}

CompressionSystem::CompressionSystem(int numParticles, float lambda)
{
    Q_ASSERT(lambda > 1);

    // generate a straight line of particles
    for(int i = 0; i < numParticles; ++i) {
        insert(new CompressionParticle(Node(i, 0), -1, randDir(), *this, lambda));
    }
}

bool CompressionSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    // terminates on disconnection
    if(!isConnected(particles)) {
        return true;
    }
#endif

    return false;
}
