//// Brian Parker
//// Note that all of my shape formation algorithms make use of "State" instead of "phase". The two are equivalent in theory, but not in programming.
//#ifndef RING_H
//#define RING_H

//#include "alg/legacy/algorithmwithflags.h"

//class LegacySystem;

//namespace Ring
//{
//enum class State {
//    Finished,
//    Leader2,
//    Follower2,
//    Set,
//    Leader,
//    Follower,
//    Idle,
//    Seed
//};


//class RingFlag : public Flag
//{
//public:
//    RingFlag();
//    RingFlag(const RingFlag& other);
//    Ring::State state;
//    bool point; //
//    bool stopper; // This algorithm also makes use of a stopper, which is a "mid-point" for the expanded ring
//    bool followIndicator;
//    int contractDir;
//};

//class Ring : public AlgorithmWithFlags<RingFlag>
//{
//public:
    
//    Ring(const State _state);
//    Ring(const Ring& other);
//    virtual ~Ring();

//    static std::shared_ptr<LegacySystem> instance(const unsigned int size, const double holeProb);

//    virtual Movement execute();
//    virtual std::shared_ptr<Algorithm> blank() const override;
//    virtual std::shared_ptr<Algorithm> clone() override;
//    virtual bool isDeterministic() const;
//    virtual bool isStatic() const;
//    virtual bool isRetired() const;

//protected:
    
    
//    int isPointedAt(); //
//    void setPoint(int _label);

//    // Functions to access the stopper
//    bool nearStopper();
//    void setStopper(bool value = true);

//    void setState(State _state);
//    bool neighborInState(int direction, State _state);
//    bool hasNeighborInState(State _state);
//    int firstNeighborInState(State _state);

//    int getMoveDir();
//    void setContractDir(const int contractDir);
//    int updatedFollowDir() const;

//    void unsetFollowIndicator();
//    void setFollowIndicatorLabel(const int label);
//    bool tailReceivesFollowIndicator() const;
//    bool followIndicatorMatchState(State _state) const;
    
//protected:
//    State state;
//    int followDir;
//    int wait;
//};
//}



//#endif // RING_H

// Defines the particle system and composing particles for the Ring Formation
// Algorithm as defined in 'An Algorithmic Framework for Shape Formation
// Problems in Self-Organizing Particle Systems' [arxiv.org/abs/1504.00744].
//
// Run with Ring(#particles, hole probability) on the simulator command line.

/** TODO
 *  1) Probably going to have to use something to do moveDir
 *  2) Set them to wait state rather than finish state
 *  3) Somehow manipulate the activate function to take into account the wait function
 *  4) Might not have to do anything with constructionDir
 *  5) moveNum = totalNum / 6
 *  6) Stop movement when the lead particle (which might need to change, unsure)
 *       reaches another particle in its moveDir path
 *  7) Once movement stops, set everything to finish state
 *
*/

#ifndef AMOEBOTSIM_ALG_RING_H
#define AMOEBOTSIM_ALG_RING_H

#include "alg/amoebotparticle.h"
#include "alg/amoebotsystem.h"

class RingParticle : public AmoebotParticle {
 public:
  enum class State {
    Seed,
    Idle,
    Follow,
    Lead,
    Finish,
    Wait,
    Follow2
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system which it belongs to, and an initial state.
  RingParticle(const Node head, const int globalTailDir,
                  const int orientation, AmoebotSystem& system, State state);

  // Executes one particle activation.
  virtual void activate();

  // Functions for altering a particle's cosmetic appearance; headMarkColor
  // (respectively, tailMarkColor) returns the color to be used for the ring
  // drawn around the head (respectively, tail) node. Tail color is not shown
  // when the particle is contracted. headMarkDir returns the label of the port
  // on which the black head marker is drawn.
  virtual int headMarkColor() const;
  virtual int headMarkDir() const;
  virtual int tailMarkColor() const;

  // Returns the string to be displayed when this particle is inspected; used
  // to snapshot the current values of this particle's memory at runtime.
  virtual QString inspectionText() const;

  // Gets a reference to the neighboring particle incident to the specified port
  // label. Crashes if no such particle exists at this label; consider using
  // hasNbrAtLabel() first if unsure.
  RingParticle& nbrAtLabel(int label) const;

  // Returns the label of the first port incident to a neighboring particle in
  // any of the specified states, starting at the (optionally) specified label
  // and continuing clockwise.
  int labelOfFirstNbrInState(std::initializer_list<State> states,
                             int startLabel = 0) const;

  // Checks whether this particle has a neighbor in any of the given states.
  bool hasNbrInState(std::initializer_list<State> states) const;

  // Returns the label of the port incident to a neighbor which is finished and
  // pointing at this particle's position as the next one to be filled; returns
  // -1 if such a neighbor does not exist.
  int constructionFinishReceiveDir() const;

  int constructionWaitReceiveDir() const;

  // Checks whether this particle is occupying the next position to be filled
  // for a finished state.
  bool canFinish() const;

  // Checks whether this particle is occupying the next position to be filled
  // for a wait state.
  bool canWait() const;

  // Sets this particle's constructionDir to point at the next position to be
  // filled as it is finishing.
  void updateConstructionDir();

  // Updates this particle's moveDir when it is a leader to traverse the current
  // surface of the forming shape counter-clockwise.
  void updateMoveDir();

  // Checks whether this particle has an immediate child in the spanning tree
  // following its tail.
  bool hasTailFollower() const;

 protected:
  State state;
  int constructionDir;
  int moveDir;
  int followDir;
  int totalNumber;
  int moveNum;

 private:
  friend class RingSystem;
};

class RingSystem : public AmoebotSystem  {
 public:
  // Constructs a system of RingParticles with an optionally specified size
  // (#particles) and hole probability. holeProb in [0,1] controls how "spread
  // out" the system is; closer to 0 is more compressed, closer to 1 is more
  // expanded.
  RingSystem(int numParticles = 200, float holeProb = 0.2);

  // Checks whether or not the system's run of the Ring formation algorithm
  // has terminated (all particles in state Finish).
  bool hasTerminated() const override;
};

#endif  // AMOEBOTSIM_ALG_RING_H
