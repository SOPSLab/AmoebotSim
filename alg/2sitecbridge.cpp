#include <algorithm> // used for find()
#include <math.h> // used for random number generation
#include <QtGlobal>
#include <set>

#include "alg/2sitecbridge.h"
#include "core/localparticle.h"

TwoSiteCBridgeParticle::TwoSiteCBridgeParticle(const Node head,
                                               const int globalTailDir,
                                               const int orientation,
                                               AmoebotSystem& system,
                                               Role role,
                                               const double lambda,
                                               const double alpha)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      role(role),
      lambda(lambda),
      alpha(alpha)
{
    q = 0; numParticleNbrs1 = 0; numSiteNbrs1 = 0; flag = false;
}

void TwoSiteCBridgeParticle::activate()
{
    if(role == Role::Particle) { // only particles perform movements; sites remain stationary
        if(isContracted()) {
            int expandDir = randDir(); // select a random neighboring location
            q = randDouble(0,1); // select a random q in (0,1)

            if(canExpand(expandDir) && !hasExpandedNeighbor()) {
                // count neighbors before expansion
                numParticleNbrs1 = neighborCount(uniqueLabels(), Role::Particle);
                numSiteNbrs1 = neighborCount(uniqueLabels(), Role::Site);

                expand(expandDir); // expand to the unoccupied position

                flag = !hasExpandedNeighbor();
            }
        }
        else { // is expanded
            // can only attempt to contract to new location if flag == TRUE and original position does not have 5 neighbors
            if(flag && (numParticleNbrs1 + numSiteNbrs1) != 5) {
                // count neighbors in new location
                int numParticleNbrs2 = occupiedLabelsNoExpandedHeads(headLabels(), Role::Particle).size();
                int numSiteNbrs2 = occupiedLabelsNoExpandedHeads(headLabels(), Role::Site).size();

                // check if the bias probability is satisfied and the locations satisfy property 1 or 2
                if((q < pow(lambda, numParticleNbrs2 - numParticleNbrs1 + alpha * (numSiteNbrs2 - numSiteNbrs1)))
                        && (checkProp1(Role::Particle) || checkProp2(Role::Particle))
                        && (checkProp1(Role::All) || checkProp2(Role::All))) {
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
    }

    return;
}

int TwoSiteCBridgeParticle::headMarkColor() const
{
    if(role == Role::Site) {
        return 0x000000;
    }
    else {
        return -1;
    }
}

QString TwoSiteCBridgeParticle::inspectionText() const
{
    QString text;
    text = "Properties:\n";
    if(role == Role::Site) {
        text += "    role = Site.";
    }
    else { // role == Role::Particle
        text += "    role = Particle,\n";
        text += "    lambda = " + QString::number(lambda) + ",\n";
        text += "    alpha = " + QString::number(alpha) + ",\n";
        text += "    q in (0,1) = " + QString::number(q) + ",\n";
        text += "    flag = " + QString::number(flag) + ".\n";

        if(isContracted()) {
            text += "Contracted properties:\n";
            text += "    #particle neighbors in first position = " + QString::number(numParticleNbrs1) + ",\n";
            text += "    #site neighbors in first position = " + QString::number(numSiteNbrs1) + ".";
        }
        else { // is expanded
            text += "Expanded properties:\n";
            text += "    #particle neighbors in first position = " + QString::number(numParticleNbrs1) + ",\n";
            text += "    #particle neighbors in second position = " + QString::number(occupiedLabelsNoExpandedHeads(headLabels(), Role::Particle).size()) + ",\n";
            text += "    #site neighbors in first position = " + QString::number(numSiteNbrs1) + ",\n";
            text += "    #site neighbors in second position = " + QString::number(occupiedLabelsNoExpandedHeads(headLabels(), Role::Site).size()) + ".";
        }
    }

    return text;
}

TwoSiteCBridgeParticle& TwoSiteCBridgeParticle::nbrAtLabel(int label) const
{
    return AmoebotParticle::nbrAtLabel<TwoSiteCBridgeParticle>(label);
}

bool TwoSiteCBridgeParticle::hasExpandedNeighbor() const
{
    for(const int label: uniqueLabels()) {
        if(hasNbrAtLabel(label) && nbrAtLabel(label).isExpanded()) {
            return true;
        }
    }

    return false;
}

int TwoSiteCBridgeParticle::neighborCount(std::vector<int> labels, const Role r) const
{
    int neighbors = 0;

    for(const int label : labels) {
        if(hasNbrAtLabel(label) && (r == Role::All || nbrAtLabel(label).role == r)) {
            ++neighbors;
        }
    }

    return neighbors;
}

bool TwoSiteCBridgeParticle::checkProp1(const Role r) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(flag); // not required by algorithm, but equivalent and cleaner for our implementation

    const std::vector<int> allLabels = uniqueLabels();
    const std::vector<int> occLabels = occupiedLabelsNoExpandedHeads(allLabels, r);

    // find the size of S
    int sizeS = 0;
    for(int label : {tailLabels()[4], headLabels()[4]}) {
        if(std::find(occLabels.begin(), occLabels.end(), label) != occLabels.end()) {
            ++sizeS;
        }
    }

    if(sizeS != 0) { // S has to be nonempty for Property 1
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

bool TwoSiteCBridgeParticle::checkProp2(const Role r) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(flag); // not required by algorithm, but equivalent and cleaner for our implementation

    const std::vector<int> occLabels = occupiedLabelsNoExpandedHeads(uniqueLabels(), r);

    // find the size of S
    int sizeS = 0;
    for(int label : {tailLabels()[4], headLabels()[4]}) {
        if(std::find(occLabels.begin(), occLabels.end(), label) != occLabels.end()) {
            ++sizeS;
        }
    }

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
            if(nbrNodeReachedViaLabel(label) != nbrNodeReachedViaLabel((label + 9) % 10)) {
                labels.push_back(label);
            }
        }

        Q_ASSERT(labels.size() == 8); // sanity check
        return labels;
    }
}

const std::vector<int> TwoSiteCBridgeParticle::occupiedLabelsNoExpandedHeads(std::vector<int> labels, const Role r) const
{
    Q_ASSERT(flag); // expanded particles with flag == TRUE can ignore heads of expanded neighbors since their flag == FALSE

    std::vector<int> occNoExpHeadLabels;
    for(const int label : labels) {
        // if the label points at the head of an expanded neighboring particle, do not include it
        if(hasNbrAtLabel(label) && (r == Role::All || nbrAtLabel(label).role == r)
                && !(nbrAtLabel(label).isExpanded() && nbrAtLabel(label).pointsAtMyHead(*this, label)))
        {
            occNoExpHeadLabels.push_back(label);
        }
    }

    return occNoExpHeadLabels;
}

// TODO: update initialization
TwoSiteCBridgeSystem::TwoSiteCBridgeSystem(int numParticles, double lambda, double alpha)
{
    Q_ASSERT(lambda > 1);
    Q_ASSERT(alpha >= 1);

    // generate a straight line of particles
    for(int i = 0; i < numParticles; ++i) {
        insert(new TwoSiteCBridgeParticle(Node(i, 0), -1, randDir(), *this, TwoSiteCBridgeParticle::Role::Particle, lambda, alpha));
    }
    // generate two sites at distance 2n/5 and 3n/5 along the line
    insert(new TwoSiteCBridgeParticle(Node(2*numParticles/5, -1), -1, randDir(), *this, TwoSiteCBridgeParticle::Role::Site, lambda, alpha));
    insert(new TwoSiteCBridgeParticle(Node(3*numParticles/5, -1), -1, randDir(), *this, TwoSiteCBridgeParticle::Role::Site, lambda, alpha));
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
