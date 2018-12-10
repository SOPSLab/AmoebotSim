#ifndef LEGACYSYSTEM_H
#define LEGACYSYSTEM_H

#include <array>
#include <deque>
#include <map>
#include <random>
#include <set>

#include "alg/legacy/algorithm.h"
#include "alg/legacy/legacyparticle.h"
#include "core/system.h"
#include "core/tile.h"

class LegacySystem : public System {
protected:
  enum class SystemState {
    Valid,
    Disconnected,
    Terminated,
    Deadlocked
  };

public:
  LegacySystem();
  virtual ~LegacySystem();

  void activate() final;
  void activateParticleAt(Node node) final;

  unsigned int size() const final;
  unsigned int numTiles() const final;
  virtual const LegacyParticle& at(int i) const;
  virtual const std::deque<Tile*>& getTiles() const final;

  unsigned int numMovements() const final;
  unsigned int numRounds() const final;
  unsigned int leaderElectionRounds() const final;
  unsigned int weakBounds() const final;
  unsigned int strongBounds() const final;

  void setWeakBound(unsigned int bound);
  void setStrongBound(unsigned int bound);

  virtual bool hasTerminated() const;

  void insertParticle(const LegacyParticle& p);

protected:
  std::array<const Flag*, 10> assembleFlags(LegacyParticle& p);
  bool handleExpansion(LegacyParticle& p, int label);
  bool handleContraction(LegacyParticle& p, int label, bool isHandoverContraction);
  void updateNumRounds(LegacyParticle* p);
  template<int n>
  static int posMod(int a);

protected:
  std::deque<LegacyParticle*> particles;
  std::map<Node, LegacyParticle*> particleMap;
  std::set<LegacyParticle*> activatedParticles;
  std::deque<LegacyParticle*> shuffledParticles;
  std::deque<Tile*> tiles;
  std::map<Node, Tile*> tileMap;

  std::mt19937 rng;

  SystemState systemState;

  unsigned int numNonStaticParticles;
  unsigned int _numMovements;
  unsigned int _numRounds;
  unsigned int _leaderElectionRounds;
  unsigned int _weakBound;
  unsigned int _strongBound;
};

template<int n>
int LegacySystem::posMod(int a) {
  return (a % n + n) % n;
}

#endif // LEGACYSYSTEM_H
