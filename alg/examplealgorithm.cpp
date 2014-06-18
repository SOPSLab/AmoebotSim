#include "alg/examplealgorithm.h"
#include "sim/particle.h"
#include "sim/system.h"

ExampleAlgorithm::ExampleAlgorithm(const Phase _phase)
    : phase(_phase),
      followDir(-1),
      distanceToTravel(3) // The line of particles should travel 3 nodes wide.
{
    // The following method conveniently sets up the outFlags.
    initFlags<ExampleFlag>();
    outFlags = castFlags<ExampleFlag>(Algorithm::outFlags);
    setPhase(_phase);
}


ExampleAlgorithm::ExampleAlgorithm(const ExampleAlgorithm& other)
    : Algorithm(other),     // Do not forget to call the constructor of the Algorithm class!
      phase(other.phase),   // And again, make sure to provide a correct copy constructor.
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
    // Create a line of particles where the right-most one is a leader.
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
    // Our algorithm works completely deterministically. Specifying this here allows the simulator to detect deadlocks.
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
        // Finished particles do nothing.
        return Movement(MovementType::Empty);
    }

    if(isExpanded()) {
        // Expanded particles contract no matter of their phase.
        if(inFlags[headContractionLabel()] == nullptr) {
            // inFlags[headContractionLabel()] denotes the flag of the node behind the tail.
            // Note that an inFlag has the value "nullptr" if there is no particle occuying the respective node.
            return Movement(MovementType::Contract, tailContractionLabel());
        } else {
            return Movement(MovementType::HandoverContract, tailContractionLabel());
        }
    } else {
        if(hasNeighborInPhase(Phase::Finished)) {
            // If a particle has a finished neighbor, it will become finished itself.
            setPhase(Phase::Finished);
            return Movement(MovementType::Idle);
        }

        if(phase == Phase::Leader) {
            // The leader travels to the right for "distanceToTravel" many steps and then finishes.
            if(distanceToTravel > 0) {
                distanceToTravel--;
                auto moveDir = determineMoveDir();
                // The mark associated to a head or tail (see below in setPhase) can be extended to show a direction.
                // If the direction is -1, the mark is drawn without an indicator for direction.
                // Otherwise, the direction must be from [0, 5] (not adhering to this will cause a crash!)
                // and the indicator will point in that direction.
                headMarkDir = moveDir;
                return Movement(MovementType::Expand, moveDir);
            } else {
                setPhase(Phase::Finished);
                return Movement(MovementType::Idle);
            }
        } else if(phase == Phase::Follower){
            // A follower knows in which direction to expand and simply does so.
            return Movement(MovementType::Expand, followDir);
        } else {//phase == Phase::Idle
            // If an idle particle sees any active particle (leader or follower), it becomes active itself.
            followDir = determineFollowDir();
            headMarkDir = followDir;
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
        // The head and the tail of a particle can be marked by a separate color.
        headMarkColor = 0x00ff00;
        tailMarkColor = 0x00ff00;
    } else if(phase == Phase::Leader) {
        headMarkColor = 0xff0000;
        tailMarkColor = 0xff0000;
    } else if(phase == Phase::Follower) {
        headMarkColor = 0x0000ff;
        tailMarkColor = 0x0000ff;
    } else {//phase == Phase::Idle
        // Setting the color to -1 will remove the mark.
        headMarkColor = -1;
        tailMarkColor = -1;
    }

    for(auto it = outFlags.begin(); it != outFlags.end(); ++it) {
        ExampleFlag& flag = *(*it);
        flag.phase = phase;
    }
}

bool ExampleAlgorithm::hasNeighborInPhase(Phase _phase)
{
    for(int label = 0; label < 10; label++) {
        if(inFlags[label] != nullptr) { // Again, note that an inFlag can be nullptr!
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
    return -1;
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

ExampleFlag::ExampleFlag()
{
}

ExampleFlag::ExampleFlag(const ExampleFlag& other)
    : Flag(other),      // Do not forget to call the constructor of the Flag class!
      phase(other.phase)// And be sure to implement correct copy constructors.
{
}
