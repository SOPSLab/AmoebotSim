#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class CompressionParticle : public AmoebotParticle
{
    friend class CompressionSystem;

public:
    CompressionParticle(const Node head,
                        const int globalTailDir,
                        const int orientation,
                        AmoebotSystem& system,
                        const float lambda);

    virtual void activate();

    virtual QString inspectionText() const;

    CompressionParticle& neighborAtLabel(int label) const;

protected:
    const float lambda;
    float q;
    int sizeS, numNeighbors, numTriBefore;
    bool flag;

private:
    bool hasExpandedNeighbor() const;
    int neighborCount(std::vector<int> labels) const;
    int triangleCount() const;
    bool checkProp1() const;
    bool checkProp2() const;
    std::vector<int> uniqueLabels() const;
};

class CompressionSystem : public AmoebotSystem
{
public:
    CompressionSystem(int numParticles = 200, float lambda = 4.0);

    virtual bool hasTerminated() const;
};

#endif // COMPRESSION_H
