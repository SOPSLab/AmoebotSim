#include <set>
#include <QtGlobal>
#include <QDebug> // used for qDebug() calls, can be taken out when done developing
#include <math.h> // used for random number generation

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
    q = 0; sizeS = 0; numNeighbors = 0; numTriBefore = 0; flag = true;
}

void CompressionParticle::activate()
{
    if(isContracted()) {
        qDebug() << "activating contracted particle";

        int expandDir = randDir(); // select a random neighboring location
        q = randFloat(0,1); // select a random q in (0,1)

        qDebug() << "expandDir =" << expandDir << ", q =" << q;

        if(canExpand(expandDir) && !hasExpandedNeighbor()) {
            qDebug() << "particle can expand and does not have an expanded neighbor";
            // compute the number of particles that will be in set S after expansion
            sizeS = 0;
            if(hasNeighborAtLabel((expandDir + 1) % 6)) {
                ++sizeS;
            }
            if(hasNeighborAtLabel((expandDir + 5) % 6)) {
                ++sizeS;
            }

            qDebug() << "after counting, sizeS =" << sizeS;

            numNeighbors = neighborCount(uniqueLabels()); // count neighbors before expansion
            numTriBefore = triangleCount(); // count triangles before expansion

            qDebug() << "numNeighbors =" << numNeighbors << ", numTriBefore =" << numTriBefore;

            expand(expandDir); // expand to the unoccupied position

            qDebug() << "particle expanded to" << expandDir;

            flag = !hasExpandedNeighbor();

            qDebug() << "flag =" << flag;
        }
    }
    else { // is expanded
        qDebug() << "activating expanded particle";

        int numTriAfter = triangleCount(); // count triangles formed in new location

        qDebug() << "numTriAfter =" << numTriAfter << ", checkProp1() =" << checkProp1() << ", checkProp2() =" << checkProp2() << ", q =" << q << ", lambda^(t'-t) =" << pow(lambda, numTriAfter - numTriBefore) << ", flag =" << flag;

        if((numNeighbors != 5) // original position cannot have five neighbors
           && (checkProp1() || checkProp2()) // these occupied locations must satisfy property 1 or 2
           && (q < pow(lambda, numTriAfter - numTriBefore)) // the bias probability is satisfied
           && flag) { // its flag was set to true when it first expanded
            qDebug() << "contracting to new position";
            contractTail(); // contract to new location
        }
        else {
            qDebug() << "contracting back to old position";
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
        int firstOccupiedIndex = -1;
        std::vector<int> labels = uniqueLabels();

        for(int i = 0; (size_t)i < labels.size(); ++i) {
            // find the first position which is occupied but the previous (clockwise) is not
            if(hasNeighborAtLabel(labels[i]) && !hasNeighborAtLabel(labels[(i + labels.size() - 1) % labels.size()])) {
                firstOccupiedIndex = i;
                break;
            }
        }
        Q_ASSERT(firstOccupiedIndex != -1); // DEBUG: sanity check

        int consecutiveCount = 0; // proceeding counter-clockwise, count the number of consecutive occupied positions
        for(int offset = 0; (size_t)offset < labels.size(); ++offset) {
            if(hasNeighborAtLabel(labels[(firstOccupiedIndex + offset) % labels.size()])) {
                ++consecutiveCount;
            }
            else {
                break;
            }
        }

        return consecutiveCount == neighborCount(uniqueLabels());
    }
    else {
        return false;
    }
}

bool CompressionParticle::checkProp2() const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(0 <= sizeS && sizeS <= 2);

    if(sizeS == 0) { // S has to be empty for Property 2
        // both nodes occupied by the particle need to have at least one neighbor
        bool hasHeadNeighbor = false, hasTailNeighbor = false;
        for(const int headLabel : headLabels()) {
            if(hasNeighborAtLabel(headLabel)) {
                hasHeadNeighbor = true;
                break;
            }
        }
        for(const int tailLabel : tailLabels()) {
            if(hasNeighborAtLabel(tailLabel)) {
                hasTailNeighbor = true;
                break;
            }
        }

        // all neighbors of the head must be consecutive
        bool hasConsecutiveHead = false;
        if(hasHeadNeighbor) {
            int firstOccupiedHeadIndex = -1;
            for(int i = 0; (size_t)i < headLabels().size(); ++i) {
                if(hasNeighborAtLabel(headLabels()[i]) && !hasNeighborAtLabel(headLabels()[(i + headLabels().size() - 1) % headLabels().size()])) {
                    firstOccupiedHeadIndex = i;
                    break;
                }
            }
            Q_ASSERT(firstOccupiedHeadIndex != -1); // DEBUG: sanity check

            int consecutiveHeadCount = 0;
            for(int offset = 0; (size_t)offset < headLabels().size(); ++offset) {
                if(hasNeighborAtLabel(headLabels()[(firstOccupiedHeadIndex + offset) % headLabels().size()])) {
                    ++consecutiveHeadCount;
                }
                else {
                    break;
                }
            }

            hasConsecutiveHead = consecutiveHeadCount == neighborCount(headLabels());
        }

        // all neighbors of the tail must be consecutive
        bool hasConsecutiveTail = false;
        if(hasTailNeighbor) {
            int firstOccupiedTailIndex = -1;
            for(int i = 0; (size_t)i < tailLabels().size(); ++i) {
                if(hasNeighborAtLabel(tailLabels()[i]) && !hasNeighborAtLabel(tailLabels()[(i + tailLabels().size() - 1) % tailLabels().size()])) {
                    firstOccupiedTailIndex = i;
                    break;
                }
            }
            Q_ASSERT(firstOccupiedTailIndex != -1); // DEBUG: sanity check

            int consecutiveTailCount = 0;
            for(int offset = 0; (size_t)offset < tailLabels().size(); ++offset) {
                if(hasNeighborAtLabel(tailLabels()[(firstOccupiedTailIndex + offset) % tailLabels().size()])) {
                    ++consecutiveTailCount;
                }
                else {
                    break;
                }
            }

            hasConsecutiveTail = consecutiveTailCount == neighborCount(tailLabels());
        }

        return hasHeadNeighbor && hasTailNeighbor && hasConsecutiveHead && hasConsecutiveTail;
    }
    else {
        return false;
    }
}

std::vector<int> CompressionParticle::uniqueLabels() const
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

    // TODO: define other termination criteria
    return false;
}
