#include "amoebotparticle.h"

AmoebotParticle::AmoebotParticle(const Node head,
                                 const int tailDir,
                                 const int orientation,
                                 std::map<Node, AmoebotParticle*>& particleMap)
    : LabelledNoCompassParticle(head, tailDir, orientation)
    , particleMap(particleMap)
{

}

int AmoebotParticle::headMarkGlobalDir() const
{
    int dir = headMarkDir();
    Q_ASSERT(-1 <= dir && dir < 6);
    if(dir == -1) {
        return -1;
    } else {
        return localToGlobalDir(dir);
    }
}

int AmoebotParticle::tailMarkGlobalDir() const
{
    int dir = tailMarkDir();
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

    Node newHead = neighboringNodeReachedViaLabel(label);
    auto it = particleMap.find(newHead);

    if(it == particleMap.end()) {
        return true;
    } else {
        AmoebotParticle* neighbor = it->second;
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

    Node newHead = neighboringNodeReachedViaLabel(label);
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
    if(label == headContractionLabel()) {
        contractHead();
    } else if(label == tailContractionLabel()) {
        contractTail();
    } else {
        Q_ASSERT(false);
    }
}

bool AmoebotParticle::hasNeighborAtLabel(int label) const
{
    Node node = neighboringNodeReachedViaLabel(label);
    return particleMap.find(node) != particleMap.end();
}

AmoebotParticle& AmoebotParticle::neighborAtLabel(int label) const
{
    Node node = neighboringNodeReachedViaLabel(label);
    auto it = particleMap.find(node);
    if(it == particleMap.end()) {
        Q_ASSERT(false);
    }

    return *((*it).second);
}

bool AmoebotParticle::hasHeadAtLabel(int label)
{
    return hasNeighborAtLabel(label) && neighborAtLabel(label).head == neighboringNodeReachedViaLabel(label);
}

bool AmoebotParticle::hasTailAtLabel(int label)
{
    if(!hasNeighborAtLabel(label)) {
        return false;
    }

    AmoebotParticle& neighbor = neighborAtLabel(label);
    if(neighbor.isContracted()) {
        return false;
    }

    if(neighbor.tail() == neighboringNodeReachedViaLabel(label)) {
        return true;
    } else {
        return false;
    }
}

void AmoebotParticle::putToken(Token* token)
{
    tokens.push_back(token);
}
