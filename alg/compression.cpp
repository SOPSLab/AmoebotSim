#include <algorithm> // used for find()
#include <math.h> // used for random number generation
#include <QtGlobal>
#include <QDebug> // used for qDebug() calls, can be taken out when done developing
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
    q = 0; sizeS = 0; numNeighbors = 0; numTriBefore = 0; flag = false;
}

void CompressionParticle::activate()
{
    if(isContracted()) {
        // qDebug() << "activating contracted particle";

        int expandDir = randDir(); // select a random neighboring location
        q = randFloat(0,1); // select a random q in (0,1)

        // qDebug() << "expandDir =" << expandDir << ", q =" << q;

        if(canExpand(expandDir) && !hasExpandedNeighbor()) {
            // qDebug() << "particle can expand and does not have an expanded neighbor";

            // count neighbors and triangles before expansion
            numNeighbors = neighborCount(uniqueLabels());
            numTriBefore = triangleCount();

            // qDebug() << "numNeighbors =" << numNeighbors << ", numTriBefore =" << numTriBefore;

            expand(expandDir); // expand to the unoccupied position

            // qDebug() << "particle expanded to" << expandDir;

            flag = !hasExpandedNeighbor();

            // qDebug() << "flag =" << flag;
        }
    }
    else { // is expanded
        // qDebug() << "activating expanded particle";

        // compute the number of particles that will be in set S after expansion
        const std::vector<int> labels = occupiedLabelsNoExpandedHeads();
        sizeS = 0;
        if(std::find(labels.begin(), labels.end(), headLabels()[0]) != labels.end()) {
            ++sizeS;
        }
        if(std::find(labels.begin(), labels.end(), headLabels()[4]) != labels.end()) {
            ++sizeS;
        }

        // qDebug() << "after counting, sizeS =" << sizeS;

        int numTriAfter = triangleCount(); // count triangles formed in new location

        // qDebug() << "numTriAfter =" << numTriAfter << ", checkProp1() =" << checkProp1() << ", checkProp2() =" << checkProp2() << ", q =" << q << ", lambda^(t'-t) =" << pow(lambda, numTriAfter - numTriBefore) << ", flag =" << flag;

        if((numNeighbors != 5) // original position cannot have five neighbors
           && (q < pow(lambda, numTriAfter - numTriBefore)) // the bias probability is satisfied
           && flag // its flag was set to true when it first expanded
           && (checkProp1() || checkProp2())) { // these occupied locations must satisfy property 1 or 2
            // qDebug() << "contracting to new position";
            contractTail(); // contract to new location
        }
        else {
            // qDebug() << "contracting back to old position";
            contractHead(); // contract back to original location
        }
    }

    return;
}

int CompressionParticle::headMarkColor() const
{
    if(isExpanded()) {
        return 0xff0000;
    }
    else {
        return -1;
    }
}

int CompressionParticle::tailMarkColor() const
{
    if(isExpanded()) {
        return 0x0000ff;
    }
    else {
        return -1;
    }
}

QString CompressionParticle::inspectionText() const
{
    QString text;
    text = "Properties:\n";
    text += "    lambda = " + QString::number(lambda) + ",\n";
    text += "    q in (0,1) = " + QString::number(q) + ",\n";
    text += "    size of S = " + QString::number(sizeS) + ",\n";
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
        text += "    satisfies property 1? = " + QString::number(checkProp1()) + ",\n";
        text += "    satisfies property 2? = " + QString::number(checkProp2()) + ".\n";
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

// NOTE: still double counts expanded particles which are entirely contained in neighborhood
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

bool CompressionParticle::checkProp1() const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(0 <= sizeS && sizeS <= 2);

    if(sizeS != 0) { // S has to be nonempty for Property 1
        const std::vector<int> allLabels = uniqueLabels();
        const std::vector<int> occLabels = occupiedLabelsNoExpandedHeads();

        // find the first position which is occupied but the previous (clockwise) is not
        int firstOccupiedIndex = -1;
        for(int i = 0; (size_t)i < allLabels.size(); ++i) {
            if(flag) { // if this expanded particle has flag == TRUE, it needs to ignore heads of adjacent expanded particles
                if(std::find(occLabels.begin(), occLabels.end(), allLabels[i]) != occLabels.end() &&
                   std::find(occLabels.begin(), occLabels.end(), allLabels[(i + allLabels.size() - 1) % allLabels.size()]) == occLabels.end()) {
                    firstOccupiedIndex = i;
                    break;
                }
            }
            else if(hasNeighborAtLabel(allLabels[i]) && !hasNeighborAtLabel(allLabels[(i + allLabels.size() - 1) % allLabels.size()])) {
                firstOccupiedIndex = i;
                break;
            }
        }
        Q_ASSERT(firstOccupiedIndex != -1); // DEBUG: sanity check

        // proceeding counter-clockwise, count the number of consecutive occupied positions
        int consecutiveCount = 0;
        for(int offset = 0; (size_t)offset < allLabels.size(); ++offset) {
            if(flag) { // again, flag == TRUE means it needs to ignore heads of adjacent expanded particles
                if(std::find(occLabels.begin(), occLabels.end(), allLabels[(firstOccupiedIndex + offset) % allLabels.size()]) != occLabels.end()) {
                    ++consecutiveCount;
                }
                else {
                    break;
                }
            }
            else {
                if(hasNeighborAtLabel(allLabels[(firstOccupiedIndex + offset) % allLabels.size()])) {
                    ++consecutiveCount;
                }
                else {
                    break;
                }
            }
        }

        const int totalNeighbors = flag ? occLabels.size() : neighborCount(uniqueLabels());
        return consecutiveCount == totalNeighbors;
    }
    else {
        return false;
    }
}

bool CompressionParticle::checkProp2() const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(0 <= sizeS && sizeS <= 2);

    const std::vector<int> occLabels = occupiedLabelsNoExpandedHeads();

    if(sizeS == 0) { // S has to be empty for Property 2
        // both nodes occupied by the particle need to have at least one neighbor
        bool hasHeadNeighbor = false, hasTailNeighbor = false;
        for(const int headLabel : headLabels()) {
            if(flag) { // flag == TRUE means this expanded particle needs to ignore heads of adjacent expanded particles
                if(std::find(occLabels.begin(), occLabels.end(), headLabel) != occLabels.end()) {
                    hasHeadNeighbor = true;
                    break;
                }
            }
            else if(hasNeighborAtLabel(headLabel)) {
                hasHeadNeighbor = true;
                break;
            }
        }
        for(const int tailLabel : tailLabels()) {
            if(flag) { // again, ignore heads of adjacent expanded particles
                if(std::find(occLabels.begin(), occLabels.end(), tailLabel) != occLabels.end()) {
                    hasTailNeighbor = true;
                    break;
                }
            }
            else if(hasNeighborAtLabel(tailLabel)) {
                hasTailNeighbor = true;
                break;
            }
        }

        // all neighbors of the head must be consecutive
        bool hasConsecutiveHead = false;
        if(hasHeadNeighbor) {
            int firstOccupiedHeadIndex = -1;
            for(int i = 0; (size_t)i < headLabels().size(); ++i) {
                if(flag) { // again, ignore heads of adjacent expanded particles
                    if(std::find(occLabels.begin(), occLabels.end(), headLabels()[i]) != occLabels.end() &&
                       std::find(occLabels.begin(), occLabels.end(), headLabels()[(i + headLabels().size() - 1) % headLabels().size()]) == occLabels.end()) {
                        firstOccupiedHeadIndex = i;
                        break;
                    }
                }
                else if(hasNeighborAtLabel(headLabels()[i]) && !hasNeighborAtLabel(headLabels()[(i + headLabels().size() - 1) % headLabels().size()])) {
                    firstOccupiedHeadIndex = i;
                    break;
                }
            }

            // DEBUG: what the occLabels are and what the headLabels are to find the issue
            if(firstOccupiedHeadIndex == -1) {
                qDebug() << "In head portion of checkProp2()";
                qDebug() << "Expanded particle has flag == " << QString::number(flag);
                QString occText = "OccNoExHeads: [";
                for(const int occLabel : occLabels) {
                    occText += QString::number(occLabel) + ", ";
                }
                occText += "].";
                qDebug() << occText;

                QString headText = "HeadLabels: [";
                for(const int hLabel : headLabels()) {
                    if(hasNeighborAtLabel(hLabel)) {
                        headText += "(" + QString::number(hLabel) + ")" + ", ";
                    }
                    else {
                        headText += QString::number(hLabel) + ", ";
                    }
                }
                headText += "].";
                qDebug() << headText;
            }
            Q_ASSERT(firstOccupiedHeadIndex != -1); // DEBUG: sanity check

            int consecutiveHeadCount = 0;
            for(int offset = 0; (size_t)offset < headLabels().size(); ++offset) {
                if(flag) { // again, ignore heads of adjacent expanded particles
                    if(std::find(occLabels.begin(), occLabels.end(), headLabels()[(firstOccupiedHeadIndex + offset) % headLabels().size()]) != occLabels.end()) {
                        ++consecutiveHeadCount;
                    }
                    else {
                        break;
                    }
                }
                else {
                    if(hasNeighborAtLabel(headLabels()[(firstOccupiedHeadIndex + offset) % headLabels().size()])) {
                        ++consecutiveHeadCount;
                    }
                    else {
                        break;
                    }
                }
            }

            // count number of head labels in occLabels
            int occHeadNeighbors = 0;
            for(const int occLabel : occLabels) {
                if(isHeadLabel(occLabel)) {
                    ++occHeadNeighbors;
                }
            }

            const int totalHeadNeighbors = flag ? occHeadNeighbors : neighborCount(headLabels());
            hasConsecutiveHead = consecutiveHeadCount == totalHeadNeighbors;
        }

        // all neighbors of the tail must be consecutive
        bool hasConsecutiveTail = false;
        if(hasTailNeighbor) {
            int firstOccupiedTailIndex = -1;
            for(int i = 0; (size_t)i < tailLabels().size(); ++i) {
                if(flag) { // again, ignore heads of adjacent expanded particles
                    if(std::find(occLabels.begin(), occLabels.end(), tailLabels()[i]) != occLabels.end() &&
                       std::find(occLabels.begin(), occLabels.end(), tailLabels()[(i + tailLabels().size() - 1) % tailLabels().size()]) == occLabels.end()) {
                        firstOccupiedTailIndex = i;
                        break;
                    }
                }
                else if(hasNeighborAtLabel(tailLabels()[i]) && !hasNeighborAtLabel(tailLabels()[(i + tailLabels().size() - 1) % tailLabels().size()])) {
                    firstOccupiedTailIndex = i;
                    break;
                }
            }

            // DEBUG: what the occLabels are and what the tailLabels are to find the issue
            if(firstOccupiedTailIndex == -1) {
                qDebug() << "In tail portion of checkProp2()";
                qDebug() << "Expanded particle has flag == " << QString::number(flag);
                QString occText = "OccNoExHeads: [";
                for(const int occLabel : occLabels) {
                    occText += QString::number(occLabel) + ", ";
                }
                occText += "].";
                qDebug() << occText;

                QString tailText = "TailLabels: [";
                for(const int tLabel : tailLabels()) {
                    if(hasNeighborAtLabel(tLabel)) {
                        tailText += "(" + QString::number(tLabel) + ")" + ", ";
                    }
                    else {
                        tailText += QString::number(tLabel) + ", ";
                    }
                }
                tailText += "].";
                qDebug() << tailText;
            }
            Q_ASSERT(firstOccupiedTailIndex != -1); // DEBUG: sanity check

            int consecutiveTailCount = 0;
            for(int offset = 0; (size_t)offset < tailLabels().size(); ++offset) {
                if(flag) { // again, ignore heads of adjacent expanded particles
                    if(std::find(occLabels.begin(), occLabels.end(), tailLabels()[(firstOccupiedTailIndex + offset) % tailLabels().size()]) != occLabels.end()) {
                        ++consecutiveTailCount;
                    }
                    else {
                        break;
                    }
                }
                else {
                    if(hasNeighborAtLabel(tailLabels()[(firstOccupiedTailIndex + offset) % tailLabels().size()])) {
                        ++consecutiveTailCount;
                    }
                    else {
                        break;
                    }
                }
            }

            // count number of tail labels in occLabels
            int occTailNeighbors = 0;
            for(const int occLabel : occLabels) {
                if(isTailLabel(occLabel)) {
                    ++occTailNeighbors;
                }
            }

            const int totalTailNeighbors = flag ? occTailNeighbors : neighborCount(tailLabels());
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

// NOTE: should only be used by expanded particles with flag == TRUE, since they know that adjacent expanded particles have flag == FALSE
const std::vector<int> CompressionParticle::occupiedLabelsNoExpandedHeads() const
{
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

    /*insert(new CompressionParticle(Node(0, 0), -1, randDir(), *this, lambda));
    insert(new CompressionParticle(Node(1, 0), -1, randDir(), *this, lambda));
    insert(new CompressionParticle(Node(-1, 0), -1, randDir(), *this, lambda));
    insert(new CompressionParticle(Node(0, 1), -1, randDir(), *this, lambda));
    insert(new CompressionParticle(Node(0, -1), -1, randDir(), *this, lambda));
    insert(new CompressionParticle(Node(1, -1), -1, randDir(), *this, lambda));
    insert(new CompressionParticle(Node(-1, 1), -1, randDir(), *this, lambda));*/
}

bool CompressionSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    // terminates on disconnection
    if(!isConnected(particles)) {
        return true;
    }
#endif

    // TODO: define other termination criteria
    return false;
}
