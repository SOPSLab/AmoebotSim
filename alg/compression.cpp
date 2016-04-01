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
                                         const float gamma)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      gamma(gamma)
{}

void CompressionParticle::activate()
{
    // select a random q in (0,1)
    int q = randFloat(0,1);
    // select a random neighboring location
    int lprime = randDir();

    // check the size of set S
    int sizeS = 0;
    if(hasNeighborAtLabel((lprime + 1) % 6)) {
        ++sizeS;
    }
    if(hasNeighborAtLabel((lprime + 5) % 6)) {
        ++sizeS;
    }
    // count neighbors before expansion
    int lNeighbors = neighborCount();
    // count triangles before expansion
    int t = triangleCount(-1);

    // if there is no neighbor at l', expand there
    if(!hasNeighborAtLabel(lprime)) {
        expand(lprime);
    }
    else {
        return;
    }

    // check if the l and lprime positions satisfy properties 1 and 2
    bool satisfiesProp1 = checkProp1(sizeS);
    bool satisfiesProp2 = checkProp2(sizeS);

    // count triangles after expansion
    int tprime = triangleCount(tailDir()); // needs dir

    if(lNeighbors != 5 && (satisfiesProp1 || satisfiesProp2) && (q < pow(gamma,tprime - t))) {
        contractTail();
    }
    else {
        contractHead();
    }

    return;
}

int CompressionParticle::headMarkColor() const
{
    return -1; // no colors are needed in this algorithm either
}

int CompressionParticle::headMarkDir() const
{
    return -1; // no head is used in this algorithm
}

int CompressionParticle::tailMarkColor() const
{
    return headMarkColor();
}

QString CompressionParticle::inspectionText() const
{

}

CompressionParticle& CompressionParticle::neighborAtLabel(int label) const
{
    return AmoebotParticle::neighborAtLabel<CompressionParticle>(label);
}

int CompressionParticle::neighborCount() const
{
    int neighbors = 0;

    if(isContracted()) {
        for(int i = 0; i < 6; ++i) {
            if(hasNeighborAtLabel(i)) {
                ++neighbors;
            }
        }
    }
    else {

    }

    return neighbors;
}

int CompressionParticle::triangleCount(const int tailDir) const
{
    int triangles = 0;

    if(tailDir == -1) {
        // particle is contracted
        for(int i = 0; i < 6; ++i) {
            if(hasNeighborAtLabel(i) && hasNeighborAtLabel((i + 1) % 6)) {
                ++triangles;
            }
        }
    }
    else {
        // particle is expanded and tail shouldn't count for triangles
        // BOOKMARK
    }

    return triangles;
}

bool CompressionParticle::checkProp1(const int S) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(0 <= S && S <= 2);

    if(S != 0) { // S has to be nonempty for Property 1
        int firstOccupiedPos = -1;
        for(int label = 0; label < 10; ++label) {
            if(hasNeighborAtLabel(label) && !hasNeighborAtLabel((label + 9) % 10)) {
                firstOccupiedPos = label;
                break;
            }
        }

        int consecutiveCount = 0;
        for(int offset = 0; offset < 10; ++offset) {
            if(hasNeighborAtLabel((firstOccupiedPos + offset) % 10)) {
                ++consecutiveCount;
            }
        }

        return consecutiveCount == neighborCount();
    }
    else {
        return false;
    }
}

bool CompressionParticle::checkProp2(const int S) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(0 <= S && S <= 2);

    if(S == 0) { // S has to be empty for Property 2

    }
    else {
        return false;
    }
}

CompressionSystem::CompressionSystem(int numParticles, float gamma)
{
    Q_ASSERT(gamma > 1); // required by algorithm

    // generate a random, connected configuration with no holes
}

bool CompressionSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    // terminates on disconnection
    if(!isConnected(particles)) {
        return true;
    }
#endif

    // define other termination criteria
}
