#include "alg/examplealgorithm.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace ExampleAlgorithm
{
ExampleAlgorithm::ExampleAlgorithm(const Phase _phase)
    : phase(_phase),
      followDir(-1),
      distanceToTravel(3) // The line of particles should travel 3 nodes wide.
{
    setPhase(_phase);
}


ExampleAlgorithm::ExampleAlgorithm(const ExampleAlgorithm& other)
    : AlgorithmWithFlags(other),    // Do not forget to call the constructor of the AlgorithmWithFlags class template!
      phase(other.phase),           // And again, make sure to provide a correct copy constructor.
      followDir(other.followDir),
      distanceToTravel(other.distanceToTravel)
{
}

ExampleAlgorithm::~ExampleAlgorithm()
{
}

std::shared_ptr<System> ExampleAlgorithm::instance(const int numParticles)
{
    // Create a line of particles where the right-most one is a leader.
    std::shared_ptr<System> system = std::make_shared<System>();
    for(int x = 0; x < numParticles; x++) {
        Phase phase = x == numParticles - 1 ? Phase::Leader : Phase::Idle;
        int orientation = randDir();
        Node position = Node(x - numParticles / 2, 0);
        int tailDir = -1;
        system->insert(Particle(std::make_shared<ExampleAlgorithm>(phase), orientation, position, tailDir));
    }
    return system; // Note that the ownership goes to the caller!
}

Movement ExampleAlgorithm::execute()
{
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

std::shared_ptr<Algorithm> ExampleAlgorithm::blank()
{
    return std::make_shared<ExampleAlgorithm>(Phase::Idle);
}

std::shared_ptr<Algorithm> ExampleAlgorithm::clone()
{
    // Use (correctly implemented) copy constructor to do the cloning.
    return std::make_shared<ExampleAlgorithm>(*this);
}

bool ExampleAlgorithm::isDeterministic() const
{
    // Our algorithm works completely deterministically. Specifying this here allows the simulator to detect deadlocks.
    return true;
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

    for(int i = 0; i < 10; i++) {
        outFlags[i].phase = phase;
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

}
