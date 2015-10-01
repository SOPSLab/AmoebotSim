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

    if(isExpanded()) {
        return false;
    }

    const Node handoverNode = neighboringNodeReachedViaLabel(label);
    auto it = particleMap.find(handoverNode);

    if(it == particleMap.end()) {
        // can expand into empty node
        return true;
    } else {
        const AmoebotParticle* neighbor = it->second;
        if(neighbor->isExpanded()) {
            // can push another particle
            return true;
        } else {
            // cannot expand
            return false;
        }
    }
}

void AmoebotParticle::expand(int label)
{
    Q_ASSERT(canExpand(label));

    const Node handoverNode = neighboringNodeReachedViaLabel(label);
    globalTailDir = (labelToGlobalDir(label) + 3) % 6;
    head = handoverNode;

    auto it = particleMap.find(handoverNode);
    if(it != particleMap.end()) {
        // push
        AmoebotParticle* neighbor = it->second;
        if(handoverNode == neighbor->head) {
            // head of neighbor is pushed
            neighbor->head = neighbor->tail();
        }
        neighbor->globalTailDir = -1;
    }
    particleMap[handoverNode] = this;
}

void AmoebotParticle::contractHead()
{
    Q_ASSERT(isExpanded());
    particleMap.erase(tail());
    head = tail();
    globalTailDir = -1;
}

void AmoebotParticle::contractTail()
{
    Q_ASSERT(isExpanded());
    particleMap.erase(tail());
    globalTailDir = -1;
}

void AmoebotParticle::contract(int label)
{
    Q_ASSERT(0 <= label && label < 10);
    Q_ASSERT(label == headContractionLabel() || label == tailContractionLabel());
    if(label == headContractionLabel()) {
        contractHead();
    } else {
        contractTail();
    }
}

bool AmoebotParticle::canHandoverContract(int label)
{
    Q_ASSERT(0 <= label && label < 10);
    return (isExpanded() && neighborAtLabel<Particle>(label).isContracted());
}

void AmoebotParticle::handoverContract(int label)
{
    Q_ASSERT(canHandoverContract(label));

    auto& neighbor = neighborAtLabel<AmoebotParticle>(label);
    if(isHeadLabel(label)) {
        particleMap[head] = &neighbor;
        neighbor.head = head;
        head = tail();
    } else {
        particleMap[tail()] = &neighbor;
        neighbor.head = tail();
    }

    neighbor.globalTailDir = labelToGlobalDir(label);
    globalTailDir = -1;
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
