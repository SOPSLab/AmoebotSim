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
    return (isContracted() && !hasNeighborAtLabel(label));
}

void AmoebotParticle::expand(int label)
{
    Q_ASSERT(canExpand(label));
    const int globalExpansionDir = localToGlobalDir(label);
    head = head.nodeInDir(globalExpansionDir);
    globalTailDir = (globalExpansionDir + 3) % 6;
    particleMap[head] = this;
}

bool AmoebotParticle::canPush(int label)
{
    Q_ASSERT(0 <= label && label < 6);
    return (isContracted() && hasNeighborAtLabel(label) && neighborAtLabel<Particle>(label).isExpanded());
}

void AmoebotParticle::push(int label)
{
    Q_ASSERT(canPush(label));

    const int globalExpansionDir = localToGlobalDir(label);
    const Node handoverNode = head.nodeInDir(globalExpansionDir);
    auto& neighbor = neighborAtLabel<Particle>(label);

    head = handoverNode;
    globalTailDir = (globalExpansionDir + 3) % 6;
    particleMap[handoverNode] = this;

    if(handoverNode == neighbor.head) {
        neighbor.head = neighbor.tail();
    }
    neighbor.globalTailDir = -1;
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

bool AmoebotParticle::canPull(int label)
{
    Q_ASSERT(0 <= label && label < 10);
    return (isExpanded() && hasNeighborAtLabel(label) && neighborAtLabel<Particle>(label).isContracted());
}

void AmoebotParticle::pull(int label)
{
    Q_ASSERT(canPull(label));

    const int globalPullDir = labelToGlobalDir(label);
    const Node handoverNode = isHeadLabel(label) ? head : tail();
    auto& neighbor = neighborAtLabel<AmoebotParticle>(label);

    if(isHeadLabel(label)) {
        head = tail();
    }

    globalTailDir = -1;
    neighbor.head = handoverNode;
    neighbor.globalTailDir = globalPullDir;
    particleMap[handoverNode] = &neighbor;
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
