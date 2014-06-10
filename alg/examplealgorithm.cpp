#include "alg/examplealgorithm.h"
#include "sim/particle.h"
#include "sim/system.h"

ExampleFlag::ExampleFlag()
{
}

ExampleFlag::ExampleFlag(const ExampleFlag& other)
    : Flag(other) // Do not forget to call the constructor of the Flag class!
{
}

ExampleAlgorithm::ExampleAlgorithm(const bool _isSeed)
    : isSeed(_isSeed),
      expandDir(0) // Initially, the expand direction should be 0.
{
    // The following method conveniently sets up the outFlags.
    initFlags<ExampleFlag>();

    // You can assign a color to each the head and the tail of a particle.
    if(isSeed) {
        headColor = 0xff0000;
        tailColor = 0x00ff00;
    } else {
        headColor = 0x0000ff;
        tailColor = -1; // -1 specifies that there should be no circle around the
    }
}


ExampleAlgorithm::ExampleAlgorithm(const ExampleAlgorithm& other)
    : Algorithm(other), // Do not forget to call the constructor of the Algorithm class!
      isSeed(other.isSeed),
      expandDir(other.expandDir)
{
    // The following method conveniently copies the outFlags, provided the copy constructor for the flags is correctly
    // implemented.
    copyFlags<ExampleFlag>(other);
}

ExampleAlgorithm::~ExampleAlgorithm()
{
    // The following method conveniently deletes the outFlags.
    deleteFlags();
}

System* ExampleAlgorithm::instance(const int size)
{
    System* system = new System();
    for(int x = 0; x < size; x++) {
        bool isSeed = (x == 0);
        int orientation = randDir();
        Node position = Node(x, 0);
        int tailDir = -1;
        system->insert(Particle(new ExampleAlgorithm(isSeed), orientation, position, tailDir));
    }
    return system; // Note that the ownership goes to the caller!
}

Algorithm* ExampleAlgorithm::clone()
{
    // Use (correctly implemented) copy constructor to do the cloning.
    return new ExampleAlgorithm(*this);
}

bool ExampleAlgorithm::isDeterministic() const
{
    return true;
}

Movement ExampleAlgorithm::execute(std::array<const Flag*, 10>& flags)
{
    /*
     * As you can see, the flags received as a parameter have type Flag*. However, we would like to have flags of type
     * ExampleFlag*. The following method provides convenient casting. Note that it does not check whether the casts are
     * valid!
     * */
    auto inFlags = castFlags<ExampleFlag>(flags);


    if(isExpanded()) {
//        if(inFlags[5] == nullptr) {
            return Movement(MovementType::Contract, tailContractionLabel());
//        } else {
            return Movement(MovementType::HandoverContract, tailContractionLabel());
//        }
    } else {
//        if(inFlags[0] == nullptr) {
//            headColor = 0xff0000;
//            tailColor = 0xff0000;
//        } else if(inFlags[3] == nullptr) {
//            headColor = 0x00ff00;
//            tailColor = 0x00ff00;
//        } else {
//            headColor = 0x0000ff;
//            tailColor = 0x0000ff;
//        }
        expandDir = (expandDir + 1) % 6;
        return Movement(MovementType::Expand, expandDir);
    }
}
