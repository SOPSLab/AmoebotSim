#include <algorithm> // used for find()
#include <math.h> // used for rng, sqrt, and floor
#include <QtGlobal>
#include <set>

#include "alg/twositeebridge.h"
#include "alg/labellednocompassparticle.h"

TwoSiteEBridgeParticle::TwoSiteEBridgeParticle(const Node head,
                                               const int globalTailDir,
                                               const int orientation,
                                               AmoebotSystem& system,
                                               Role role,
                                               const float explambda,
                                               const float complambda,
                                               const bool flag)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      role(role),
      explambda(explambda),
      complambda(complambda),
      flag(flag)
{
    lambda = explambda; q = 0; numNbrs1 = 0;
}

void TwoSiteEBridgeParticle::activate()
{
    if(role == Role::Particle) { // only particles perform movements; sites remain stationary
        if(isContracted()) {
            int expandDir = randDir(); // select a random neighboring location
            q = randFloat(0,1); // select a random q in (0,1)

            // check for particles in neighborhood with updated lambdas (expansion -> compression)
            if(hasNeighborWithLambda(complambda)) {
                lambda = complambda;
            }

            if(canExpand(expandDir) && !hasExpandedNeighbor()) {
                // count neighbors before expansion
                numNbrs1 = neighborCount(uniqueLabels(), Role::All, true);

                expand(expandDir); // expand to the unoccupied position

                flag = !hasExpandedNeighbor();
            }
        }
        else { // is expanded
            // can only attempt to contract to new location if flag == TRUE and original position does not have 5 neighbors
            if(flag && numNbrs1 != 5) {
                // count neighbors in new location
                int numNbrs2 = occupiedLabelsNoExpandedHeads(headLabels(), Role::All, true).size();

                // check if the bias probability is satisfied and the locations satisfy property 1 or 2
                if((q < pow(lambda, numNbrs2 - numNbrs1))
                        && (checkProp1(Role::Particle) || checkProp2(Role::Particle))
                        && (checkProp1(Role::All) || checkProp2(Role::All))) {
                    contractTail(); // contract to new location

                    // if there is a new site in the neighborhood, set its flag to TRUE and update this particle's lambda
                    for(const int label : uniqueLabels()) {
                        if(hasNeighborAtLabel(label) && neighborAtLabel(label).role == Role::Site && !neighborAtLabel(label).flag) {
                            neighborAtLabel(label).flag = true;
                            lambda = complambda;
                        }
                    }
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

int TwoSiteEBridgeParticle::headMarkColor() const
{
    if(role == Role::Site) {
        return 0x000000;
    }
    else { // role == Role::Particle
        if(lambda == explambda) { // in expansion mode
            return 0x0000ff;
        }
        else { // in compression mode
            return 0xff0000;
        }
    }
}

int TwoSiteEBridgeParticle::tailMarkColor() const
{
    return headMarkColor();
}

QString TwoSiteEBridgeParticle::inspectionText() const
{
    QString text;
    text = "Properties:\n";
    if(role == Role::Site) {
        text += "    role = Site,\n";
        text += "    flag = " + QString::number(flag) + ".";
    }
    else { // role == Role::Particle
        text += "    role = Particle,\n";
        text += "    lambda = " + QString::number(lambda) + ",\n";
        text += "    q in (0,1) = " + QString::number(q) + ",\n";
        text += "    flag = " + QString::number(flag) + ".\n";

        if(isContracted()) {
            text += "Contracted properties:\n";
            text += "    #neighbors in first position = " + QString::number(numNbrs1) + ".";
        }
        else { // is expanded
            text += "Expanded properties:\n";
            text += "    #neighbors in first position = " + QString::number(numNbrs1) + ",\n";
            text += "    #neighbors in second position = " + QString::number(occupiedLabelsNoExpandedHeads(headLabels(), Role::All, true).size()) + ".";
        }
    }

    return text;
}

TwoSiteEBridgeParticle& TwoSiteEBridgeParticle::neighborAtLabel(int label) const
{
    return AmoebotParticle::neighborAtLabel<TwoSiteEBridgeParticle>(label);
}

bool TwoSiteEBridgeParticle::hasExpandedNeighbor() const
{
    for(const int label: uniqueLabels()) {
        if(hasNeighborAtLabel(label) && neighborAtLabel(label).isExpanded()) {
            return true;
        }
    }

    return false;
}

bool TwoSiteEBridgeParticle::hasNeighborWithLambda(const int val) const
{
    for(const int label : uniqueLabels()) {
        if(hasNeighborAtLabel(label) && neighborAtLabel(label).lambda == val) {
            return true;
        }
    }

    return false;
}

int TwoSiteEBridgeParticle::neighborCount(std::vector<int> labels, const Role r, const bool countNewSites) const
{
    int neighbors = 0;

    for(const int label : labels) {
        if(hasNeighborAtLabel(label) && (r == Role::All || neighborAtLabel(label).role == r)
                && !(neighborAtLabel(label).role == Role::Site && !countNewSites && !neighborAtLabel(label).flag)) {
            ++neighbors;
        }
    }

    return neighbors;
}

bool TwoSiteEBridgeParticle::checkProp1(const Role r) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(flag); // not required by algorithm, but equivalent and cleaner for our implementation

    const std::vector<int> allLabels = uniqueLabels();
    const std::vector<int> occLabels = occupiedLabelsNoExpandedHeads(allLabels, r, false);

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

bool TwoSiteEBridgeParticle::checkProp2(const Role r) const
{
    Q_ASSERT(isExpanded());
    Q_ASSERT(flag); // not required by algorithm, but equivalent and cleaner for our implementation

    const std::vector<int> occLabels = occupiedLabelsNoExpandedHeads(uniqueLabels(), r, false);

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

const std::vector<int> TwoSiteEBridgeParticle::uniqueLabels() const
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

const std::vector<int> TwoSiteEBridgeParticle::occupiedLabelsNoExpandedHeads(std::vector<int> labels, const Role r, const bool countNewSites) const
{
    Q_ASSERT(flag); // expanded particles with flag == TRUE can ignore heads of expanded neighbors since their flag == FALSE

    std::vector<int> occNoExpHeadLabels;
    for(const int label : labels) {
        // if the label points at the head of an expanded neighboring particle, do not include it
        if(hasNeighborAtLabel(label) && (r == Role::All || neighborAtLabel(label).role == r)
                && !(neighborAtLabel(label).role == Role::Site && !countNewSites && !neighborAtLabel(label).flag)
                && !(neighborAtLabel(label).isExpanded() && neighborAtLabel(label).pointsAtMyHead(*this, label)))
        {
            occNoExpHeadLabels.push_back(label);
        }
    }

    return occNoExpHeadLabels;
}

TwoSiteEBridgeSystem::TwoSiteEBridgeSystem(int numParticles, float explambda, float complambda, float siteDistance)
{
    // first insert the anchor site at the center of the hexagon
    insert(new TwoSiteEBridgeParticle(Node(0,0), -1, randDir(), *this, TwoSiteEBridgeParticle::Role::Site, 0, 0, true));

    // generate a hexagon
    int posx, posy;
    for(int i = 2; i <= numParticles; ++i) {
        int layer = 1;
        int position = i - 1;
        while(position - (6 * layer) >= 0) {
            position -= 6 * layer;
            ++layer;
        }

        switch(position / layer) {
            case 0: {
                posx = layer;
                posy = (position % layer) - layer;
                if(position % layer == 0) {posx -= 1; posy += 1;} // addresses a corner case
                break;
            }
            case 1: {posx = layer - (position % layer); posy = position % layer; break;}
            case 2: {posx = -1 * (position % layer); posy = layer; break;}
            case 3: {posx = -1 * layer; posy = layer - (position % layer); break;}
            case 4: {posx = (position % layer) - layer; posy = -1 * (position % layer); break;}
            case 5: {posx = (position % layer); posy = -1 * layer; break;}
        }

        insert(new TwoSiteEBridgeParticle(Node(posx, posy), -1, randDir(), *this, TwoSiteEBridgeParticle::Role::Particle, explambda, complambda, false));
    }

    // lastly, insert the site the system is exploring and bridging to
    insert(new TwoSiteEBridgeParticle(Node(floor(siteDistance*sqrt(numParticles)), 0), -1, randDir(), *this, TwoSiteEBridgeParticle::Role::Site, 0, 0, false));
}

bool TwoSiteEBridgeSystem::hasTerminated() const
{
    // this simulation shouldn't terminate on disconnection, since the site to be explored is always initially disconnected
    return false;
}
