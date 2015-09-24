#ifndef LEGACYPARTICLE_H
#define LEGACYPARTICLE_H

#include <array>
#include <memory>

#include "alg/legacy/algorithm.h"
#include "alg/legacy/movement.h"
#include "alg/labellednocompassparticle.h"

class LegacyParticle : public LabelledNoCompassParticle
{
public:
    LegacyParticle(std::shared_ptr<Algorithm> _algorithm,
                   const int _orientation,
                   const Node _head,
                   const int _tailDir = -1);
    virtual ~LegacyParticle();

    Movement executeAlgorithm(std::array<const Flag*, 10>& inFlag);

    void apply();
    void discard();

    const Flag* getFlagForNodeInDir(const Node node, const int dir);
    std::shared_ptr<const Algorithm> getAlgorithm() const;

    bool algorithmIsDeterministic() const;

    bool isStatic() const;

    virtual int headMarkColor() const;
    virtual int headMarkGlobalDir() const;
    virtual int tailMarkColor() const;
    virtual int tailMarkGlobalDir() const;

    virtual std::array<int, 18> borderColors() const;
    virtual int borderDir(const int dir) const;
    virtual std::array<int, 6> borderPointColors() const;
    virtual int borderPointDir(const int dir) const;

protected:
    std::shared_ptr<Algorithm> algorithm;
    std::shared_ptr<Algorithm> newAlgorithm;
};

#endif // LEGACYPARTICLE_H
