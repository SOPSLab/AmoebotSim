#include <QtGlobal>

#include <set>
#include <vector>

#include "alg/convexhull.h"

ConvexHullParticle::ConvexHullParticle(const Node head, const int globalTailDir,
                                 const int orientation, AmoebotSystem& system,
                                 State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    parent(-1),
    moveDir(-1),
    delta(std::vector<std::vector<int> >(6)),
    distance(std::vector<int>(6))  // NE, N, NW, SW, S, SE
    {
        for ( int i = 0 ; i < 6 ; i++ )
           delta[i].resize(6);

        delta[0] = {-1, 0, 1, 1, 0, -1}; // E
        delta[1] = {-1, -1, 0, 1, 1, 0}; // NE
        delta[2] = {0, -1, -1, 0, 1, 1}; // NW
        delta[3] = {1, 0, -1, -1, 0, 1}; // W
        delta[4] = {1, 1, 0, -1, -1, 0}; // SW
        delta[5] = {0, 1, 1, 0, -1, -1}; // SE
    }

void ConvexHullParticle::activate() {
    if (state == State::Object) {
        return;
    }

    else if (state == State::LeaderStart) {
        state = State::LeaderMove;
        moveDir = 0;
    }

    else if (state == State::LeaderMove) {
        if (isExpanded()) {
            contractTail();
        }
        else {

            // Check whether the convex hull approximation can be updated
            for(int hp = 0; hp < 6; hp++) {
                if (distance[hp] == 0 && (hasTileAtLabel(hp) or hasTileAtLabel((hp + 1) % 6) or hasTileAtLabel((hp + 2) % 6) or hasTileAtLabel((hp + 5) % 6))) {
                    distance[hp]++;
                }
            }

            // Walk around the boundary in clockwise fashion
            int checkDirs[6] = {0, 5, 4, 1, 2, 3}; // If, e.g., pointing E, check in this order: E, SE, SW, NE, NW, W
            for(auto &checkDir : checkDirs) {
                if (!hasTileAtLabel((moveDir + checkDir) % 6) && hasTileAtLabel((moveDir + checkDir + 5) % 6)) {
                    moveDir = (moveDir + checkDir) % 6;
                    for(int i = 0; i < 6; i++) distance[i] = distance[i] + delta[moveDir][i];
                    expand(moveDir);
                    return;
                }
            }
        }

    }

}

int ConvexHullParticle::headMarkColor() const {
  switch(state) {
    case State::Object:         return 0x000000;
    case State::LeaderStart:    return 0x00ff00;
    case State::LeaderWait:     return 0x00ff00;
    case State::LeaderMove:     return 0x00ff00;
    case State::Done:           return 0x0000ff;
  }

  return -1;
}

int ConvexHullParticle::headMarkDir() const {
  if (state == State::Object or state == State::Done) {
    return -1;
  }
  return -1;
}

int ConvexHullParticle::tailMarkColor() const {
  return headMarkColor();
}

QString ConvexHullParticle::inspectionText() const {
  QString text;
  text += "head: (" + QString::number(head.x) + ", " + QString::number(head.y) +
    ")\n";
  text += "orientation: " + QString::number(orientation) + "\n";
  text += "globalTailDir: " + QString::number(globalTailDir) + "\n";
  text += "state: ";
  text += [this](){
    switch(state) {
    case State::Object:   return "object";
    case State::Done:   return "done";
    default:            return "no state";
    }
  }();
  text += "\n";

  return text;
}

std::vector<int> ConvexHullParticle::getConvexHullApproximate() const {
    std::vector<int> value(distance);
    return value;
}

ConvexHullSystem::ConvexHullSystem(int numParticles, int numTiles, float holeProb) {
  Q_ASSERT(numParticles > 0);
  Q_ASSERT(numTiles > 0);
  Q_ASSERT(0 <= holeProb && holeProb <= 1);

  insert(new Tile(Node(0, 0)));

  std::set<Node> occupied;
  occupied.insert(Node(0, 0));

  std::set<Node> candidates;
  for (int i = 0; i < 6; ++i) {
    candidates.insert(Node(0, 0).nodeInDir(i));
  }

  // Add object tiles
  int countTiles = 0;
  while (countTiles < numTiles && !candidates.empty()) {
    // Pick random candidate.
    std::set<Node>::const_iterator it(candidates.begin());
    advance(it,randInt(0,candidates.size()));
    Node randomCandidate = *it;
    candidates.erase(it);

    occupied.insert(randomCandidate); // node actually gets occupied only with a certain probability, but shall not be considered again

    // Add this candidate as a tile if not a hole.
    if (randBool(1.0f - holeProb)) {

      insert(new Tile(randomCandidate));
      ++countTiles;

      // Add new candidates.
      for (int i = 0; i < 6; ++i) {
        auto neighbor = randomCandidate.nodeInDir(i);
        if (occupied.find(neighbor) == occupied.end()) {
          candidates.insert(neighbor);
        }
      }
    }
  }

  // Find max occupied node
  Node maxNode = Node(0,0);
  for(auto &tile : tiles) {
      if (maxNode < tile->node) {
        maxNode = tile->node;
      }
   }

    // Place leader particle
    // To display the convex hull correctly, we set its direction to 0...
    insert(new ConvexHullParticle(Node(maxNode.x, maxNode.y + 1), -1, 0, *this,
                              ConvexHullParticle::State::LeaderStart));

}

bool ConvexHullSystem::hasTerminated() const {
  #ifdef QT_DEBUG
    if (!isConnected(particles)) {
      return true;
    }
  #endif

  for (auto p : particles) {
    auto hp = dynamic_cast<ConvexHullParticle*>(p);
    if (hp->state != ConvexHullParticle::State::Done) {
      return false;
    }
  }

  return true;
}
