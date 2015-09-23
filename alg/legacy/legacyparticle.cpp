#include "legacyparticle.h"

LegacyParticle::LegacyParticle(std::shared_ptr<Algorithm> _algorithm,
                               const int _orientation,
                               const Node head,
                               const int tailDir)
    : LabelledNoCompassParticle(head, tailDir, _orientation)
    , algorithm(_algorithm)
{
    Q_ASSERT(_algorithm != nullptr);
}

LegacyParticle::~LegacyParticle()
{

}

Movement LegacyParticle::executeAlgorithm(std::array<const Flag*, 10>& inFlag)
{
    Q_ASSERT(newAlgorithm == nullptr);
    Q_ASSERT(!algorithm->isStatic());
    newAlgorithm = algorithm->clone();
    return newAlgorithm->execute(inFlag);
}

void LegacyParticle::apply()
{
    Q_ASSERT(newAlgorithm != nullptr);
    algorithm = newAlgorithm;
    newAlgorithm.reset();
}

void LegacyParticle::discard()
{
    Q_ASSERT(newAlgorithm != nullptr);
    newAlgorithm.reset();
}

const Flag* LegacyParticle::getFlagForNodeInDir(const Node node, const int dir)
{
    return algorithm->flagAt(labelOfNeighboringNodeInGlobalDir(node, dir));
}

std::shared_ptr<const Algorithm> LegacyParticle::getAlgorithm() const{
  return std::const_pointer_cast<const Algorithm>(algorithm);
}

bool LegacyParticle::algorithmIsDeterministic() const
{
    return algorithm->isDeterministic();
}

bool LegacyParticle::isStatic() const
{
    return algorithm->isStatic();
}

int LegacyParticle::headMarkColor() const
{
    return algorithm->headMarkColor;
}

int LegacyParticle::headMarkDir() const
{
    Q_ASSERT(-1 <= algorithm->headMarkDir && algorithm->headMarkDir <= 5);
    if(algorithm->headMarkDir == -1) {
        return -1;
    } else {
        return (orientation + algorithm->headMarkDir) % 6;
    }
}

int LegacyParticle::tailMarkColor() const
{
    return algorithm->tailMarkColor;
}

int LegacyParticle::tailMarkDir() const
{
    Q_ASSERT(-1 <= algorithm->tailMarkDir && algorithm->tailMarkDir <= 5);
    if(algorithm->tailMarkDir == -1) {
        return -1;
    } else {
        return (orientation + algorithm->tailMarkDir) % 6;
    }
}

std::array<int, 18> LegacyParticle::borderColors() const
{
    std::array<int, 18> rearranged;

    for(unsigned int i = 0; i < 18; i++) {
        rearranged[borderDir(i)] = algorithm->borderColors[i];
    }

    return rearranged;
}

int LegacyParticle::borderDir(const int dir) const
{
    Q_ASSERT(-1 <= dir && dir <= 17);
    if(dir == -1) {
        return -1;
    } else {
        return (3 * orientation + dir) % 18;
    }
}

std::array<int, 6> LegacyParticle::borderPointColors() const
{
    std::array<int, 6> rearranged;

    for(unsigned int i = 0; i < 6; i++) {
        rearranged[borderPointDir(i)] = algorithm->borderPointColors[i];
    }

    return rearranged;
}

int LegacyParticle::borderPointDir(const int dir) const
{
    Q_ASSERT(-1 <= dir && dir <= 5);
    if(dir == -1) {
        return -1;
    } else {
        return (orientation + dir) % 6;
    }
}
