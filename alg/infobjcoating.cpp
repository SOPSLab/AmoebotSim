/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/infobjcoating.h"

#include <set>

InfObjCoatingParticle::InfObjCoatingParticle(const Node head,
                                             const int globalTailDir,
                                             const int orientation,
                                             AmoebotSystem &system, State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    moveDir(-1) {}

void InfObjCoatingParticle::activate() {
  if (isExpanded()) {
    if (state == State::Follower) {
      // If an expanded follower has no child or inactive neighbor, contract.
      if (!hasFollowerChild() && !hasNbrInState({State::Inactive})) {
        contractTail();
        return;
      }
    } else if (state == State::Leader) {
      // If an expanded leader holds a complaint token and has no child or
      // inactive neighbor, contract and consume token.
      if (hasToken<ComplaintToken>() && !hasNbrInState({State::Inactive})
          && !hasFollowerChild()) {
        contractTail();
        takeToken<ComplaintToken>();
        return;
      }
    }
  } else {  // Particle is contracted.
    if (state == State::Inactive) {
      // Inactive particles need to first join the spanning tree.
      if (hasObjectNbr()) {
        state = State::Leader;
        moveDir = nextSurfaceDir();
        return;
      } else if (hasNbrInState({State::Leader, State::Follower})) {
        state = State::Follower;
        moveDir = labelOfFirstNbrInState({State::Leader, State::Follower});
        return;
      }
    } else if (state == State::Follower) {
      if (hasObjectNbr()) {
        // If a follower has followed its spanning tree to the surface, become a
        // leader, removing follow direction and calculating move direction.
        state = State::Leader;
        moveDir = nextSurfaceDir();
        return;
      } else if (hasTailAtLabel(moveDir)) {
        // If a follower's parent is expanded, handover expand with it. Update
        // moveDir to continue to point at the parent after the handover.
        auto nbr = nbrAtLabel(moveDir);
        int nbrContractDir = nbrDirToDir(nbr, (nbr.tailDir() + 3) % 6);
        push(moveDir);
        moveDir = nbrContractDir;
        return;
      }
    } else if (state == State::Leader) {
      // If has a follower child, generate a complaint token if not holding one.
      if (hasFollowerChild() && !hasToken<ComplaintToken>()) {
        putToken(std::make_shared<ComplaintToken>());
      }

      // Only act if holding a complaint token.
      if (hasToken<ComplaintToken>()) {
        moveDir = nextSurfaceDir();
        if (!hasNbrAtLabel(moveDir)) {
          // If there is no particle ahead, expand and consume complaint token.
          expand(moveDir);
          takeToken<ComplaintToken>();
          return;
        } else if (hasTailAtLabel(moveDir)) {
          // If there is an expanded particle ahead, handover expand and consume
          // complaint token.
          push(moveDir);
          takeToken<ComplaintToken>();
          return;
        } else {
          // Cannot expand or handover expand; attempt to forward complaint.
          InfObjCoatingParticle& nbr = nbrAtLabel(moveDir);
          if (nbr.state == State::Leader && !nbr.hasToken<ComplaintToken>()) {
            nbr.putToken(takeToken<ComplaintToken>());
          }
          return;
        }
      }
    }
  }
}

int InfObjCoatingParticle::headMarkColor() const {
  if (hasToken<ComplaintToken>()) {
    return 0xffaa00;
  }

  switch(state) {
    case State::Inactive: return -1;
    case State::Follower: return 0x0000ff;
    case State::Leader:   return 0xff0000;
  }

  return -1;
}

int InfObjCoatingParticle::headMarkDir() const {
  return (state == State::Leader || state == State::Follower) ? moveDir : -1;
}

int InfObjCoatingParticle::tailMarkColor() const {
  return headMarkColor();
}

QString InfObjCoatingParticle::inspectionText() const {
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
                      + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Local Info:\n";
  text += "  state: ";
  text += [this](){
    switch(state) {
    case State::Inactive: return "inactive\n";
    case State::Follower: return "follower\n";
    case State::Leader:   return "leader\n";
    default:              return "no state\n";
    }
  }();
  text += "  moveDir: " + QString::number(moveDir) + "\n";
  text += "  complaint: " + QString::number(hasToken<ComplaintToken>()) + "\n";

  return text;
}

InfObjCoatingParticle& InfObjCoatingParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<InfObjCoatingParticle>(label);
}

int InfObjCoatingParticle::labelOfFirstNbrInState(
    std::initializer_list<State> states, int startLabel) const {
  auto prop = [&](const InfObjCoatingParticle& p) {
    for (auto state : states) {
      if (p.state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<InfObjCoatingParticle>(prop, startLabel);
}

bool InfObjCoatingParticle::hasNbrInState(std::initializer_list<State> states)
    const {
  return labelOfFirstNbrInState(states) != -1;
}

int InfObjCoatingParticle::nextSurfaceDir() const {
  Q_ASSERT(state == State::Leader);

  int dir = labelOfFirstObjectNbr();
  while (hasObjectAtLabel(dir)) {
    dir = (dir + 5) % 6;
  }

  return dir;
}

bool InfObjCoatingParticle::hasFollowerChild() const {
  auto prop = [&](const InfObjCoatingParticle& p) {
    return p.state == State::Follower
        && (isContracted() ? pointsAtMyHead(p, p.dirToHeadLabel(p.moveDir))
                           : pointsAtMyTail(p, p.dirToHeadLabel(p.moveDir)));
  };

  return labelOfFirstNbrWithProperty<InfObjCoatingParticle>(prop) != -1;
}

InfObjCoatingSystem::InfObjCoatingSystem(uint numParticles, double sparseness) {
  Q_ASSERT(numParticles > 0);
  Q_ASSERT(0 <= sparseness && sparseness <= 1);

  std::set<Node> objNodes;  // Nodes occupied by object.

  // Instantiate the object as a single line with turns. "Infinite" in this case
  // just means the object's surface is longer than the number of particles.
  Node objPos;
  while (objNodes.size() < numParticles * 2) {
    // Insert a new object particle at the given position.
    insert(new Object(objPos));
    objNodes.insert(objPos);

    // Calculate the next object position, avoiding 'tunnels'. Do this using
    // offsets: 5 is down-right, 0 is right, 1 is up-right.
    int offset = (randInt(2, 5) + 3) % 6;
    objPos = objPos.nodeInDir(offset);
  }

  const Node root = *std::next(objNodes.begin(), 0);

  // Construct a forest structure of particles connected to the surface. Begin
  // with unoccupied positions above/adjacent to the surface as candidates.
  std::vector<Node> candidates;
  std::vector<double> l1dists;
  for (auto objPos = objNodes.rbegin(); objPos != objNodes.rend(); ++objPos) {
    // Want some forest structure, so only include sqrt(n) positions on surface.
    if (candidates.size() > sqrt(numParticles)) {
      break;
    }

    for (int dir = 1; dir <= 2; ++dir) {
      const Node node = objPos->nodeInDir(dir);
      if (objNodes.find(node) == objNodes.end()
          && std::find(candidates.begin(), candidates.end(), node) ==
          candidates.end()) {
        candidates.push_back(node);
        l1dists.push_back(L1Dist(node, root));
      }
    }
  }

  // Add all particles.
  uint particlesAdded = 1;
  while (particlesAdded < numParticles) {
    int index = randInt(0, candidates.size());

    std::vector<double> probs = probabilityWeights(l1dists, sparseness);

    if (randBool(probs[index]/std::accumulate(probs.begin(), probs.end(), 0.0)))
    {
      Node nextParticle = candidates[index];
      insert(new InfObjCoatingParticle(nextParticle, -1, randDir(), *this,
                                       InfObjCoatingParticle::State::Inactive));
      particlesAdded++;
      candidates.erase(candidates.begin()+index);
      l1dists.erase(l1dists.begin()+index);
      InfObjCoatingParticle tmp = InfObjCoatingParticle(nextParticle, -1, 0,
                                                        *this,
                                                        InfObjCoatingParticle::\
                                                        State::Inactive);

      // Create new set of candidate nodes from the unoccupied positions
      // adjacent and above nodes occupied by newly added particles.
      for (int dir = 1; dir <= 2; ++dir) {
        const Node node = nextParticle.nodeInDir(dir);
        if (objNodes.find(node) == objNodes.end() &&
            std::find(candidates.begin(), candidates.end(), node) ==
            candidates.end() && !tmp.hasNbrAtLabel(dir)) {
          candidates.push_back(node);
          l1dists.push_back(L1Dist(node, root));
        }
      }
    }
  }
}

bool InfObjCoatingSystem::hasTerminated() const {
  // Algorithm is terminated if all particles are on the surface (leaders) and
  // have contracted.
  for (auto p : particles) {
    auto iocp = dynamic_cast<InfObjCoatingParticle*>(p);
    if ((iocp->state != InfObjCoatingParticle::State::Leader) ||
        iocp->hasToken<InfObjCoatingParticle::ComplaintToken>()) {
      return false;
    }
  }

  return true;
}

int InfObjCoatingSystem::L1Dist(Node p, Node root) {
  if (p.x >= root.x && p.y >= root.x) {
    return abs(p.x - root.x) + abs(p.y - root.y);
  }
  else if (p.x <= root.x && p.y <= root.y) {
    return abs(p.x - root.x) + abs(p.y - root.y);
  }
  else {
    if (abs(p.x - root.x) >= abs(p.y - root.y)) {
      return abs(p.x - root.x);
    }
    else {
      return abs(p.y - root.y);
    }
  }
}

std::vector<double> InfObjCoatingSystem::probabilityWeights(std::vector<double>
                                                             dists, double
                                                             sparseness) {

  int n = dists.size();

  if ( std::equal(dists.begin() + 1, dists.end(), dists.begin()) ) {
    for (int i = 0; i < n; i++) {
      dists[i] = 0.5;
    }
    return dists;
  }

  double expon_factor = pow(100, sparseness);
  for (int i = 0; i < n; i++) {
    dists[i] = pow(dists[i], expon_factor);
  }

  if (sparseness < .5) {
    for (int k = 0; k < n; k++) {
      dists[k] = 1/dists[k];
    }
  } else if (sparseness > .5) {
    sparseness = 1 - sparseness;
  }

  double min_map_bound = 0 + sparseness;
  double max_map_bound = 1 - sparseness;

  double min = *std::min_element(dists.begin(), dists.end());
  double max = *std::max_element(dists.begin(), dists.end());
  for (int k = 0; k < n; k++) {
    dists[k] = ((dists[k]-min)/(max-min)) * (max_map_bound - min_map_bound) +
        min_map_bound;
  }

  return dists;
}
