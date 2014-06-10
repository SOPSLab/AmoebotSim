#ifndef EXAMPLEALGORITHM_H
#define EXAMPLEALGORITHM_H

#include "alg/algorithm.h"
#include "sim/system.h"

class ExampleFlag : public Flag
{
public:
    ExampleFlag();
    ExampleFlag(const ExampleFlag& other);
};

class ExampleAlgorithm : public Algorithm
{
public:
    ExampleAlgorithm();
    ExampleAlgorithm(const ExampleAlgorithm& other);
    virtual ~ExampleAlgorithm();

    static System instance();
    virtual Algorithm* clone();
    virtual bool isDeterministic() const;

protected:
    virtual Movement execute(std::array<const Flag*, 10>& flags);
};

#endif // EXAMPLEALGORITHM_H
