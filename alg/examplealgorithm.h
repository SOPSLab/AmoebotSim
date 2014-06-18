/*
 * This is an example showing how to implement algorithms in AmoebotSim.
 *
 * An algorithm encapsulates the state of a particle, what information it can share using flags, and how it behaves.
 * By defining member variables in an algorithm class, one defines the information that should be part of the state of
 * a particle. Note that our formal model requires that the state space is constant in size. You have to make sure for
 * yourself that the algorithm you implement satisfies this requirement.
 *
 * An algorithm has to extend the abstract class Algorithm. This class automatically keeps track of some information
 * about the particle and provides methods to its subclasses for convenience.  For example, it provides the following:
 *      - direction of the tail relative to the orientation of a particle (tailDir), -1 if particle is contracted
 *      - whether a particle is expanded or not (isExpanded, isContracted)
 *      - what labels are the head / tail labels (headLabels, tailLabels)
 *      - what labels correspond to a head / tail contraction (headContractionLabel, tailContractionLabel)
 *      - what direction an edge with a specific label points in (labelToDir)
 *
 * A flag encapsulates the information shared by a particle with another particle. A flag has to extend the abstract
 * class Flag. Again, flags automatically contain some information about the "sending" particle, for example:
 *      - direction of the edge leaving the "sending" particle the flag was set for (dir)
 *      - direction of the tail relative to the orientation of the "sending" particle (tailDir)
 * Each algorithm has an array of "outFlags", that is the information set for the neighboring particle to read.
 *
 * The behavior of a particle is encoded in the "execute" method. As its only parameter this method gets the information
 * set by the neighboring particles (flags). In this method, the member variables (i.e., the state of a particle) can be
 * changed arbitrarily. The return value of the method describes the movement the particle should execute. In contrast
 * to the formal model, at this point an algorithm can only specify a one or no movement to be executed. The
 * MovementType enum encodes the movements allowed in the Amoebot model as well as a value "Empty" to denote that the
 * transition function for the particle should map to the empty set. Note that there is a difference between idle and
 * empty: Idle means that a particle does not want to move but wants to change its state. Empty means that, from its
 * current point of view, the particle is done. Accordingly, changes on the member variables of an algorithm will be
 * reset if the Empty movement is returned. Similarly, changes will be reset if a movement is impossible (collisions,
 * expansions of expanded particles, ...). So corresponding to the formal model, changes to the state of a particle can
 * only be applied if the associated movement succeeds (the action is enabled). If all particles in a system return
 * "Empty" as their movement, the simulator will detect that the algorithm terminated.
 *
 * The comments in the code below give more details on the implementation of algorithms. The example is a simplified
 * version of the Spanning Forest Algorithms where a line of particles moves a certain number of nodes to the right.
 * */

#ifndef EXAMPLEALGORITHM_H
#define EXAMPLEALGORITHM_H

#include "alg/algorithm.h"

class ExampleFlag;
class System;

class ExampleAlgorithm : public Algorithm
{
public:
    // The three phases from the spanning forest algorithm together with a new "Finished" phase.
    enum class Phase {
        Finished,
        Leader,
        Follower,
        Idle
    };

public:
    // Particles can be initialized differently. This can also be used to generate instances with a seed particle.
    ExampleAlgorithm(const Phase _phase);
    ExampleAlgorithm(const ExampleAlgorithm& other);
    virtual ~ExampleAlgorithm();

    /*
     * This static method generates an instance for this algorithm. It is used in the script interface
     * (scipt/scriptinterface.h). In general, the functions for generating instances should be static methods of the
     * algorithm the instances are intended for. All methods of the ScriptInterface object are available in the console
     * during runtime. The script interface is also responsible to set up the particle system when the simulator is
     * started. The implementation in script/scriptinterface.h should be self-explanatory.
     * Note, that in the current simulator all particles in a system have to have the same type of algorithm and also
     * all flags have to be of the same type. You have to make sure that this requirement is satisfied. The simulator
     * does not check it and will most probably fail in very unpleasent and undebuggable ways if the requirement is not
     * satisfied.
     * */
    static System* instance(const int size);

    /*
     * This method has to be implemented and has to correctly copy all member variables of an algorithm. The easiest way
     * to achieve this is to implement a correct copy constructor and use this to implement cloning, as is done in this
     * example algorithm.
     * */
    virtual Algorithm* clone();

    /*
     * In principle, algorithms can be probabilistic. Return true if this algorithm is fully deterministic and false
     * otherwise. The simulator will then work accordingly. For example, for deterministic algorithms the simulator can
     * detect whether a system is in a deadlock. In a nondeterministic algorithm, one of the particles included in what
     * seems to be a deadlock at one point in time might act differently later on, so that the "deadlock" resolves.
     * */
    virtual bool isDeterministic() const;

protected:
    // This method essentially determines the behavior of a particle. See description above.
    virtual Movement execute(std::array<const Flag*, 10>& flags);

    /* What the following methods do should be quite self-explanatory. However, their implementation contains some
     * relevant comments.
     * */
    void setPhase(Phase _phase);
    bool hasNeighborInPhase(Phase _phase);
    int determineMoveDir();
    int determineFollowDir();

protected:
    /*
     * These arrays represent the ingoing and outgoing flags of a particles. It is not a must to have these as members,
     * though. The outFlags array actually shadows a definition of an array in the Algorithm class of the same name, and
     * the inFlags are always provided to the execute method. However, it is convenenient to have these in order to
     * reduce the number of parameters in the protected functions above. The values are updated (mostly) in the execute
     * method.
     * */
    std::array<const ExampleFlag*, 10> inFlags;
    std::array<ExampleFlag*, 10> outFlags;

    /*
     * phase and followDir are defined as usually in the spanning forest approach. distanceToTravel specifies the number
     * of nodes the line of cells moves.
     * */
    Phase phase;
    int followDir;
    int distanceToTravel;
};

/*
 * The specific flag class for this algorithm defines the data that can be shared between particles. It has to extend
 * the Flag class.
 * */
class ExampleFlag : public Flag
{
public:
    ExampleFlag();
    ExampleFlag(const ExampleFlag& other);

    // A particle should tell its neighbors, in what phase it is.
    ExampleAlgorithm::Phase phase;
};

#endif // EXAMPLEALGORITHM_H
