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
    moveDir(-1){}

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
            int pointerDir;
            hasTileAtLabel(moveDir) ? pointerDir = 1 : pointerDir = 5;

            for(int i = 0; i < 6; i++) {
                printf("%i\n", i);
                if (!hasTileAtLabel(moveDir) && hasTileAtLabel((moveDir+5)%6)) {
                    expand(moveDir);
                    return;
                }
                moveDir = (moveDir + pointerDir) % 6;
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
  int tiles = 0;
  while (tiles < numTiles && !candidates.empty()) {
    // Pick random candidate.
    std::set<Node>::const_iterator it(candidates.begin());
    advance(it,randInt(0,candidates.size()));
    Node randomCandidate = *it;
    candidates.erase(it);

    occupied.insert(randomCandidate);

    // Add this candidate as a tile if not a hole.
    if (randBool(1.0f - holeProb)) {
      insert(new Tile(randomCandidate));
      ++tiles;

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
  for(auto node : occupied) {
      if (maxNode < node) {
        maxNode = node;
      }
   }

  printf("%i %i\n", maxNode.x, maxNode.y);

   // Place leader particle
   insert(new ConvexHullParticle(Node(maxNode.x, maxNode.y + 1), -1, randDir(), *this,
                              ConvexHullParticle::State::LeaderStart));

   printf("%i %i\n", maxNode.x , maxNode.y + 1);

}

bool ConvexHullSystem::hasTerminated() const {
  #ifdef QT_DEBUG
    if (!isConnected(particles)) {
      printf("test");
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
