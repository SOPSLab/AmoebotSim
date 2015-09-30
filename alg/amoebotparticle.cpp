#include "amoebotparticle.h"

AmoebotParticle::AmoebotParticle(const Node head,
                                 const int tailDir,
                                 const int orientation,
                                 std::map<Node, AmoebotParticle*>& particleMap)
    : LabelledNoCompassParticle(head, tailDir, orientation),
      particleMap(particleMap)
{

}

int AmoebotParticle::headMarkGlobalDir() const
{
    const int dir = headMarkDir();
    Q_ASSERT(-1 <= dir && dir < 6);
    if(dir == -1) {
        return -1;
    } else {
        return localToGlobalDir(dir);
    }
}

int AmoebotParticle::tailMarkGlobalDir() const
{
    const int dir = tailMarkDir();
    Q_ASSERT(-1 <= dir && dir < 6);
    if(dir == -1) {
        return -1;
    } else {
        return localToGlobalDir(dir);
    }
}

int AmoebotParticle::headMarkDir() const
{
    return -1;
}

int AmoebotParticle::tailMarkDir() const
{
    return -1;
}

bool AmoebotParticle::canExpand(int label)
{
    Q_ASSERT(0 <= label && label < 6);

    if(globalTailDir != -1) {
        return false;
    }

    const Node newHead = neighboringNodeReachedViaLabel(label);
    auto it = particleMap.find(newHead);

    if(it == particleMap.end()) {
        return true;
    } else {
        const AmoebotParticle* neighbor = it->second;
        if(neighbor->head == newHead) {
            return false;
        } else {
            return true;
        }
    }
}

bool AmoebotParticle::expand(int label)
{
    Q_ASSERT(0 <= label && label < 6);

    if(globalTailDir != -1) {
        return false;
    }

    const Node newHead = neighboringNodeReachedViaLabel(label);
    auto it = particleMap.find(newHead);

    if(it == particleMap.end()) {
        // expand into empty node
        globalTailDir = (labelToGlobalDir(label) + 3) % 6;
        head = newHead;
        particleMap[newHead] = this;
        return true;
    } else {
        AmoebotParticle* neighbor = it->second;
        if(neighbor->head == newHead) {
            // blocked
            return false;
        } else {
            // successful push
            globalTailDir = (labelToGlobalDir(label) + 3) % 6;
            head = newHead;
            neighbor->globalTailDir = -1;
            particleMap[newHead] = this;
            return true;
        }
    }
}

void AmoebotParticle::contractHead()
{
    particleMap.erase(tail());
    head = tail();
    globalTailDir = -1;
}

void AmoebotParticle::contractTail()
{
    particleMap.erase(tail());
    globalTailDir = -1;
}

void AmoebotParticle::contract(int label)
{
    Q_ASSERT(label == headContractionLabel() || label == tailContractionLabel());
    if(label == headContractionLabel()) {
        contractHead();
    } else if(label == tailContractionLabel()) {
        contractTail();
    }
}

bool AmoebotParticle::hasNeighborAtLabel(int label) const
{
    const Node neighboringNode = neighboringNodeReachedViaLabel(label);
    return particleMap.find(neighboringNode) != particleMap.end();
}

bool AmoebotParticle::hasHeadAtLabel(int label)
{
    return hasNeighborAtLabel(label) && neighborAtLabel<Particle>(label).head == neighboringNodeReachedViaLabel(label);
}

bool AmoebotParticle::hasTailAtLabel(int label)
{
    if(!hasNeighborAtLabel(label)) {
        return false;
    }

    const auto& neighbor = neighborAtLabel<Particle>(label);
    if(neighbor.isContracted()) {
        return false;
    }

    if(neighbor.tail() == neighboringNodeReachedViaLabel(label)) {
        return true;
    } else {
        return false;
    }
}

void AmoebotParticle::putToken(std::shared_ptr<Token> token)
{
    tokens.push_back(token);
}
