#include "alg/examplealgorithm.h"
#include "sim/particle.h"
#include "sim/system.h"

ExampleFlag::ExampleFlag()
{
}

ExampleFlag::ExampleFlag(const ExampleFlag& other)
    : Flag(other)
{
}

ExampleAlgorithm::ExampleAlgorithm()
{
    initFlags<ExampleFlag>();
}

ExampleAlgorithm::ExampleAlgorithm(const ExampleAlgorithm& other)
    : Algorithm(other)
{
    copyFlags<ExampleFlag>(other);
}

ExampleAlgorithm::~ExampleAlgorithm()
{
    deleteFlags();
}

System ExampleAlgorithm::instance()
{
    System system;
    for(int x = 0; x < 6; x++) {
        system.insert(Particle(new ExampleAlgorithm(), 0, Node(x, 0), -1));
    }
    return system;
}

Algorithm* ExampleAlgorithm::clone()
{
    return new ExampleAlgorithm(*this);
}

bool ExampleAlgorithm::isDeterministic() const
{
    return true;
}

Movement ExampleAlgorithm::execute(std::array<const Flag*, 10>& flags)
{
    auto inFlags = castFlags<ExampleFlag>(flags);

    if(isExpanded()) {
        if(inFlags[5] == nullptr) {
            return Movement(MovementType::Contract, tailContractionLabel());
        } else {
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        }
    } else {
        if(inFlags[0] == nullptr) {
            headColor = 0xff0000;
            tailColor = 0xff0000;
        } else if(inFlags[3] == nullptr) {
            headColor = 0x00ff00;
            tailColor = 0x00ff00;
        } else {
            headColor = 0x0000ff;
            tailColor = 0x0000ff;
        }
        return Movement(MovementType::Expand, 0);
    }
}
