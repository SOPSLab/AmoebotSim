#include "alg/examplealgorithm.h"
#include "sim/particle.h"
#include "sim/system.h"

ExampleFlag::ExampleFlag()
{
}

ExampleFlag::ExampleFlag(const ExampleFlag& other)
    : Flag(other), // Do not forget to call the constructor of the Flag class!
      phase(other.phase)
{
}

ExampleAlgorithm::ExampleAlgorithm(const Phase _phase)
    : phase(_phase),
      followDir(-1),
      distanceToTravel(3)
{
    // The following method conveniently sets up the outFlags.
    initFlags<ExampleFlag>();
    outFlags = castFlags<ExampleFlag>(Algorithm::outFlags);
    setPhase(_phase);
}


ExampleAlgorithm::ExampleAlgorithm(const ExampleAlgorithm& other)
    : Algorithm(other), // Do not forget to call the constructor of the Algorithm class!
      phase(other.phase),
      followDir(other.followDir),
      distanceToTravel(other.distanceToTravel)
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
        Phase phase = x == size - 1 ? Phase::Leader : Phase::Idle;
        int orientation = randDir();
        Node position = Node(x - size / 2, 0);
        int tailDir = -1;
        system->insert(Particle(new ExampleAlgorithm(phase), orientation, position, tailDir));
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
     * valid! The array of flags is stored in a member variable so that it can be accessed easily in other methods.
     * */
    inFlags = castFlags<ExampleFlag>(flags);
    outFlags = castFlags<ExampleFlag>(Algorithm::outFlags);

    if(phase == Phase::Finished) {
        return Movement(MovementType::Empty);
    }

    if(isExpanded()) {
        if(inFlags[headContractionLabel()] == nullptr) {
            return Movement(MovementType::Contract, tailContractionLabel());
        } else {
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        }
    } else {

        if(hasNeighborInPhase(Phase::Finished)) {
            setPhase(Phase::Finished);
            return Movement(MovementType::Idle);
        }

        if(phase == Phase::Leader) {
            if(distanceToTravel > 0) {
                distanceToTravel--;
                auto moveDir = determineMoveDir();
                return Movement(MovementType::Expand, moveDir);
            } else {
                setPhase(Phase::Finished);
                return Movement(MovementType::Idle);
            }
        } else if(phase == Phase::Follower){
            return Movement(MovementType::Expand, followDir);
        } else {//phase == Phase::Idle
            followDir = determineFollowDir();
            if(followDir != -1) {
                setPhase(Phase::Follower);
                return Movement(MovementType::Idle);
            } else {
                return Movement(MovementType::Empty);
            }
        }
    }
}

void ExampleAlgorithm::setPhase(Phase _phase)
{
    phase = _phase;
    if(phase == Phase::Finished) {
        headColor = 0x00ff00;
        tailColor = 0x00ff00;
    } else if(phase == Phase::Leader) {
        headColor = 0xff0000;
        tailColor = 0xff0000;
    } else if(phase == Phase::Follower) {
        headColor = 0x0000ff;
        tailColor = 0x0000ff;
    } else {//phase == Phase::Idle
        headColor = -1;
        tailColor = -1;
    }

    for(auto it = outFlags.begin(); it != outFlags.end(); ++it) {
        ExampleFlag& flag = *(*it);
        flag.phase = phase;
    }
}

bool ExampleAlgorithm::hasNeighborInPhase(Phase _phase)
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr) {
            const ExampleFlag& flag = *inFlags[label];
            if(flag.phase == _phase) {
                return true;
            }
        }
    }
    return false;
}

int ExampleAlgorithm::determineMoveDir()
{
    Q_ASSERT(isContracted());
    for(int label = 0; label < 6; label++) {
        if(inFlags[label] != nullptr) {
            return (labelToDir(label) + 3) % 6;
        }
    }
    Q_ASSERT(false);
}

int ExampleAlgorithm::determineFollowDir()
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr) {
            const ExampleFlag& flag = *inFlags[label];
            if(flag.phase == Phase::Leader || flag.phase == Phase::Follower) {
                return labelToDir(label);
            }
        }
    }
    return -1;
}
