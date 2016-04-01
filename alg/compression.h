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
                        const float gamma);

    virtual void activate();

    virtual int headMarkColor() const;
    virtual int headMarkDir() const;
    virtual int tailMarkColor() const;

    virtual QString inspectionText() const;

    CompressionParticle& neighborAtLabel(int label) const;

protected:
    // properties
    const float gamma;

private:
    int neighborCount() const;
    int triangleCount(const int tailDir) const;
    bool checkProp1(const int S) const;
    bool checkProp2(const int S) const;
};

class CompressionSystem : public AmoebotSystem
{
public:
    CompressionSystem(int numParticles = 200, float gamma = 4);

    virtual bool hasTerminated() const;
};

#endif // COMPRESSION_H
