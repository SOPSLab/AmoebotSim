#include <QtGlobal>

#include <set>
#include <vector>

#include "alg/convexhull.h"

ConvexHullParticle::ConvexHullParticle(const Node head, const int globalTailDir,
                                 const int orientation, AmoebotSystem& system,
                                 State state)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    parentDir(-1),
    moveDir(0),
    delta(std::vector<std::vector<int> >(6)),
    distance(std::vector<int>(6)),  // NE, N, NW, SW, S, SE
    completed(std::vector<int>(6)),
    turns_1(0),
    turns_2(0),
    preHull(false),
    onHull(false),
    successor(-1),
    predecessor(-1)
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

    if (state == State::Leader) {
        if (isExpanded()) {
            if (!hasChild() and !hasNeighborInState({State::Idle})) {
                contractTail();
            }
            else {
                pullChildIfPossible();
            }
        }

        else {

            // Check if already completed Phase 1

            int sumCompleted = 0;
            for (auto& i : completed) sumCompleted += i;

            if (sumCompleted != 6) {
                // Phase 1: Walk around the boundary in clockwise fashion
                int checkDirs[6] = {0, 5, 4, 1, 2, 3}; // If, e.g., pointing E, check in this order: E, SE, SW, NE, NW, W
                for(auto &checkDir : checkDirs)
                {
                    if (!hasTileAtLabel((moveDir + checkDir) % 6) && hasTileAtLabel((moveDir + checkDir + 5) % 6))
                    {
                        moveDir = (moveDir + checkDir) % 6;
                        break;
                    }
                }

                // Check if move is possible
                if (!hasNbrAtLabel(moveDir) or nbrInDirIsContracted(moveDir)) {
                    updateCompleted(moveDir);
                    updateDistances(moveDir);

                    // Perform the move
                    if (!hasNbrAtLabel(moveDir)) {
                        expand(moveDir);
                    }
                    else if (nbrInDirIsContracted(moveDir)) {
                        swapWithFollowerInDir(moveDir);
                    }
                }
            }

            else {
                // Phase 2: Follow the convex hull

                if (!onHull) {
                    // First node on the convex hull. MoveDir points to first half plane
                    moveDir = (moveDir + 5) % 6;
                    onHull = true;
                }
                else if (distance[moveDir] == 0) {
                    // The robot performs one turn
                    moveDir = (moveDir + 5) % 6;
                    turns_1++;
                    turns_2++;
                }

                // Reset first termination criterium, if the particle in front is not a contracted hull particle
                if (!(hasHullParticleInDir(moveDir) && nbrInDirIsContracted(moveDir))) {
                    turns_1 = 0;
                }

                // Reset second termination criterium, if there is a non-hull particle in its neighborhood
                for (int i = 0; i< 6; i++) {
                    if (hasNbrAtLabel(i) && !hasHullParticleInDir(i)) turns_2 = 0;
                }

                if ((turns_1 == 7 or turns_2 == 7) && hasTileAtLabel((moveDir+5) % 6)) {
                    state = State::LeaderWait;
                    return;
                }

                // Otherwise: Move along convex hull

                // Check if move is possible
                if (!hasNbrAtLabel(moveDir) or nbrInDirIsContracted(moveDir)) {

                    updateDistances(moveDir);

                    // Perform the move
                    if(!hasNbrAtLabel(moveDir)) {
                        expand(moveDir);
                    }
                    else if (nbrInDirIsContracted(moveDir)){
                        swapWithFollowerInDir(moveDir);
                    }
                }
            }
        }
    }

    else if (state == State::Idle) {
        if (hasNeighborInState({State::Leader})) {
            parentDir = labelOfFirstNbrInState({State::Leader});
            state = State::Follower;
        }
        else if (hasNeighborInState({State::Follower})) {
            parentDir = labelOfFirstNbrInState({State::Follower});
            state = State::Follower;
        }
    }

    else if (state == State::Follower) {
        if (isExpanded()) {
            if (!hasChild() and !hasNeighborInState({State::Idle})) {
                contractTail();
            }
            else {
                pullChildIfPossible();
            }
            // Contraction performed and preHull particle
            if (isContracted() && preHull) {
                onHull = true;
                preHull = false;
            }
        }
        else {
            if (nbrInDirIsInState(parentDir, {State::LeaderWait, State::FollowerWait})) {
                state = State::FollowerWait;
            }
            else if(!onHull) { // Only nonhull particles are allowed to push
                pushParentIfPossible();
            }
        }
    }

    else if (state == State::LeaderWait) {
        if (allChildrenDone()) {

            // First, check whether there are enough particles
            if (hasNbrAtLabel(moveDir)) {
                // Set successor and predecessor pointers
                successor = moveDir;
                if (hasHullParticleInDir((moveDir+3) % 6)) {
                    predecessor = (moveDir+3) % 6;
                }
                else {
                    Q_ASSERT(hasHullParticleInDir((moveDir+4) % 6));
                    predecessor = (moveDir + 4) % 6;
                }
                state = State::LeaderOrtho;
            }
            else {
                state = State::LeaderDone;
            }
        }
    }

    else if (state == State::FollowerWait) {
        if (allChildrenDone()) state = State::FollowerWait2;
    }

    else if (state == State::FollowerWait2) {
        if (nbrInDirIsInState(parentDir, {State::LeaderDone, State::FollowerDone})) {
            state = State::FollowerDone;
        }
        else if (nbrInDirIsInState(parentDir, {State::LeaderOrtho, State::FollowerOrtho})) {

            if (onHull) {
                // Set successor and predecessor pointers
                successor = parentDir;
                if (hasHullParticleInDir((parentDir+3) % 6)) {
                    predecessor = (parentDir+3) % 6;
                }
                else {
                    Q_ASSERT(hasHullParticleInDir((parentDir+4) % 6));
                    predecessor = (parentDir + 4) % 6;
                }
                parentDir = -1;
            }

            state = State::FollowerOrtho;
        }
    }

    else if (state == State::FollowerOrtho) {
        if (!hasNeighborInState({State::FollowerWait2})) {
            if (onHull) {
                // Hull particle
                if (isContracted()) {
                    // Check whether move is possible
                    if (successor == ((predecessor + 2) % 6) && !hasTileAtLabel(((predecessor + 1) % 6))) {
                        // Update successor and predecessor pointers of neighbors
                        int dir = ((predecessor + 1) % 6);
                        ConvexHullParticle& predParticle = nbrAtLabel<ConvexHullParticle>(predecessor);
                        ConvexHullParticle& succParticle = nbrAtLabel<ConvexHullParticle>(successor);
                        predParticle.successor = (predParticle.successor + 5) % 6;
                        succParticle.predecessor = (succParticle.predecessor + 1) % 6;

                        if (!hasNbrAtLabel(dir)) {
                            // Simply move in expansion direction
                            expand(dir);
                            successor = (successor + 1) % 6;
                            predecessor = (predecessor + 5) % 6;
                        }
                        else {
                            // Neighbor in expansion direction, swap with it
                            ConvexHullParticle& neighbor = nbrAtLabel<ConvexHullParticle>(dir);
                            Q_ASSERT(neighbor.isContracted());
                            Q_ASSERT(neighbor.state == State::FollowerOrtho);
                            Q_ASSERT(neighbor.onHull == false);

                            neighbor.onHull = true;
                            neighbor.successor = dirToNbrDir(neighbor, ((successor + 1) % 6));
                            neighbor.predecessor = dirToNbrDir(neighbor, ((predecessor + 5) % 6));

                            onHull = false;
                            successor = -1;
                            predecessor = -1;
                            parentDir = dir;
                        }
                    }
                }
                else {
                    // Check contraction

                    auto propertyCheck = [&](const ConvexHullParticle& p) {
                      return (p.state == State::FollowerOrtho
                             && p.onHull == false
                             && pointsAtMyTail(p, p.parentDir));
                    };

                    if (labelOfFirstNbrWithProperty<ConvexHullParticle>(propertyCheck) == -1) {
                        contractTail();
                    }
                }
            }
            else {
                // Non-hull particle
                if (isContracted() && hasTailAtLabel(parentDir)) {
                    ConvexHullParticle& nbr = nbrAtLabel<ConvexHullParticle>(parentDir);
                    int contractionDir = nbrDirToDir(nbr, (nbr.tailDir() + 3) % 6);
                    push(parentDir);
                    parentDir = contractionDir;
                }
                else if (isExpanded()) {

                    auto propertyCheck = [&](const ConvexHullParticle& p) {
                      return (p.state == State::FollowerOrtho
                             && p.onHull == false
                             && pointsAtMyTail(p, p.parentDir));
                    };

                    if (labelOfFirstNbrWithProperty<ConvexHullParticle>(propertyCheck) == -1) {
                        contractTail();
                    }
//                    else {
//                        pullChildIfPossible();
//                    }
                }
            }
        }
    }


}

void ConvexHullParticle::updateDistances(int dir) {
    for(int i = 0; i < 6; i++) distance[i] = std::max(0, distance[i] + delta[dir][i]);
}

void ConvexHullParticle::updateCompleted(int dir) {
    // Resets completed to 0, if a halfplane gets pushed
    // Otherwise, it sets all half-planes to which the leaders distance is 0 after the move to 1
    if (distance[dir] == 0 or distance[((dir + 5) % 6)] == 0) std::fill(completed.begin(), completed.end(), 0);
    else {
        for (int i = 0; i < 6; i++) {
            if (distance[i] + delta[dir][i] == 0) completed[i] = 1;
        }
    }
}

bool ConvexHullParticle::hasNeighborInState(std::initializer_list<State> states) const {
  return labelOfFirstNbrInState(states) != -1;
}

bool ConvexHullParticle::nbrInDirIsInState(int dir, std::initializer_list<State> states) const {
    Q_ASSERT(0 <= dir && dir < 6);
    Q_ASSERT(hasNbrAtLabel(dir));

    const ConvexHullParticle& p = nbrAtLabel<ConvexHullParticle>(dir);
    for (auto state : states) {
      if (p.state == state) return true;
    }
    return false;
}

bool ConvexHullParticle::nbrInDirIsContracted(int dir) const {
    Q_ASSERT(0 <= dir && dir < 6);
    Q_ASSERT(hasNbrAtLabel(dir));
    ConvexHullParticle& nbr = nbrAtLabel<ConvexHullParticle>(dir);
    return nbr.isContracted();
}

bool ConvexHullParticle::hasHullParticleInDir(int dir) const {
    Q_ASSERT(0 <= dir && dir < 10);
    if (hasNbrAtLabel(dir) ) {
        ConvexHullParticle& nbr = nbrAtLabel<ConvexHullParticle>(dir);
        if (nbr.onHull == true) return true;
    }
    return false;
}

int ConvexHullParticle::labelOfFirstNbrInState(std::initializer_list<State> states, int startLabel) const {
  auto propertyCheck = [&](const ConvexHullParticle& p) {
    for (auto state : states) {
      if (p.state == state) {
        return true;
      }
    }
    return false;
  };

  return labelOfFirstNbrWithProperty<ConvexHullParticle>(propertyCheck,
                                                           startLabel);
}

void ConvexHullParticle::swapWithFollowerInDir(int dir) {
    Q_ASSERT(0 <= dir && dir < 6);
    Q_ASSERT(hasNbrAtLabel(dir));
    Q_ASSERT(nbrInDirIsInState(dir, {State::Idle, State::Follower}));
    Q_ASSERT(nbrInDirIsContracted(dir));

    ConvexHullParticle& nbr = nbrAtLabel<ConvexHullParticle>(dir);

    // Update Follower
    nbr.state = State::Leader;

    nbr.moveDir = dirToNbrDir(nbr, moveDir);

    nbr.parentDir = -1;

    int orientOffset = nbrDirToDir(nbr, 0);
    for(int i = 0; i < 6; i++) {
        nbr.distance[i] = distance[(i+orientOffset) % 6];
    }

    for(int i = 0; i < 6; i++) {
        nbr.completed[i] = completed[(i+orientOffset) % 6];
    }

    nbr.turns_1 = turns_1;
    nbr.turns_2 = turns_2;

    nbr.onHull = onHull;

    // Update self
    state = State::Follower;
    moveDir = 0;
    parentDir = dir;
    std::fill(distance.begin(), distance.end(), 0);
    std::fill(completed.begin(), completed.end(), 0);
    turns_1 = 0;
    turns_2 = 0;

    // Update pointer to leader (for visualization)
    dynamic_cast<ConvexHullSystem&>(system).leader = &nbr;
}

bool ConvexHullParticle::hasChild() const {
    auto propertyCheck = [&](const ConvexHullParticle& p) {
      return p.state == State::Follower &&
             ((isContracted() && pointsAtMyHead(p, p.dirToHeadLabel(p.parentDir))) or
              (isExpanded() && pointsAtMyTail(p, p.dirToHeadLabel(p.parentDir))));
    };

    return labelOfFirstNbrWithProperty<ConvexHullParticle>(propertyCheck) != -1;
}

bool ConvexHullParticle::allChildrenDone() const {
    auto propertyCheck = [&](const ConvexHullParticle& p) {
      return ((p.state == State::Follower or p.state == State::FollowerWait)
             && pointsAtMe(p, p.dirToHeadLabel(p.parentDir)));
    };

    return labelOfFirstNbrWithProperty<ConvexHullParticle>(propertyCheck) == -1;
}

void ConvexHullParticle::pullChildIfPossible() {
    Q_ASSERT(isExpanded());

    // First, check if there is a non-hull particle to pull
    auto propertyCheck = [&](const ConvexHullParticle& p) {
      return p.state == State::Follower &&
             p.onHull == false &&
             pointsAtMyTail(p, p.parentDir) &&
             p.isContracted();
    };

    int dir = labelOfFirstNbrWithProperty<ConvexHullParticle>(propertyCheck);

    // If not, check if there is a hull particle to pull
    if (dir == -1) {
        auto propertyCheck = [&](const ConvexHullParticle& p) {
          return p.state == State::Follower &&
                 p.onHull == true &&
                 pointsAtMyTail(p, p.parentDir) &&
                 p.isContracted();
        };

        dir = labelOfFirstNbrWithProperty<ConvexHullParticle>(propertyCheck);
    }

    if (dir != -1) {
        ConvexHullParticle& p = nbrAtLabel<ConvexHullParticle>(dir);
        int contractDir = (tailDir() + 3) % 6;

        // If the p is also adjacent to head (and not only to tail), then dir might be a head label
        // Since we want to pull the tail, we redefine dir according to p's parent dir and make it a tail label
        int localDir = (nbrDirToDir(p, p.parentDir) + 3) % 6;
        int pullLabel = dirToTailLabel(localDir);

        pull(pullLabel);
        p.parentDir = dirToNbrDir(p, contractDir);

        if (onHull && !p.onHull) {
            p.preHull = true;
        }
    }
}

void ConvexHullParticle::pushParentIfPossible() {
    if (hasTailAtLabel(parentDir)) {

        ConvexHullParticle& nbr = nbrAtLabel<ConvexHullParticle>(parentDir);
        int contractionDir = nbrDirToDir(nbr, (nbr.tailDir() + 3) % 6);

        push(parentDir);
        parentDir = contractionDir;

        if (!onHull && nbr.onHull) {
            preHull = true;
        }
        else if (nbr.preHull) {
            nbr.onHull = true;
            nbr.preHull = false;
        }
    }
}

int ConvexHullParticle::headMarkColor() const {
  if ((state == State::Follower or state == State::FollowerOrtho) && onHull) return 0xffff00;
  else if ((state == State::Follower or state == State::FollowerOrtho)  && !onHull) return 0x0000ff;
  else if (state == State::Leader or state == State::LeaderWait or state == State::LeaderOrtho) return 0xff0000;
  else if (state == State::FollowerDone or state == State::LeaderDone) return 0x00ff00;
  else if (state == State::FollowerWait) return 0xe5d3c3;
  else if (state == State::FollowerWait2) return 0x7c3a00;
  else return -1;
}

int ConvexHullParticle::headMarkDir() const {
  if (state == State::Leader) {
    return moveDir;
  }
  else if (state == State::LeaderOrtho) {
      return successor;
  }
  else if (state == State::Follower or state == State::FollowerDone) {
      return parentDir;
  }
  else if (state == State::FollowerOrtho && onHull) {
      return successor;
  }
  else if (state == State::FollowerOrtho && !onHull) {
      return parentDir;
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

    // Add object tiles
    insert(new Tile(Node(0, 0)));

    std::set<Node> considered; //contains all nodes that have been considered already
    considered.insert(Node(0, 0));

    std::set<Node> candidates;
    for (int i = 0; i < 6; ++i) candidates.insert(Node(0, 0).nodeInDir(i));

    std::set<Node> tilePositions; //contains all nodes occupied by tiles
    tilePositions.insert(Node(0, 0));

    while ((int) tilePositions.size() < numTiles && !candidates.empty()) {
        // Pick random candidate.
        std::set<Node>::const_iterator it(candidates.begin());
        advance(it,randInt(0,candidates.size()));
        Node randomCandidate = *it;
        candidates.erase(it);

        considered.insert(randomCandidate);

        // Add this candidate as a tile only if not a hole.
        if (randBool(1.0f - holeProb)) {

            // Add this candidate only if afterwards no empty node in its neighborhood would have two partitions

            bool allEmptyNeighborsOnePartition = true;
            for (int i = 0; i < 6; ++i) {
                auto neighbor = randomCandidate.nodeInDir(i);
                // Neighbor empty?
                if (tilePositions.find(neighbor) == tilePositions.end()) {
                    // Neighborhood of tiles only one partition?
                    int changes = 0;
                    bool lastOccupied = true;
                    for (int j = 1; j < 6; ++j) {
                        bool curOccupied = (not (tilePositions.find(neighbor.nodeInDir(((i + 3 + j) % 6))) == tilePositions.end()));
                        if (lastOccupied ^ curOccupied) changes++;
                        lastOccupied = curOccupied;
                    }
                    if (changes > 2) {
                        // More than one partition in neighborhood
                        allEmptyNeighborsOnePartition = false;
                    }
                }
            }

            if (allEmptyNeighborsOnePartition) {

                insert(new Tile(randomCandidate));
                tilePositions.insert(randomCandidate);

                // Add new candidates.
                for (int i = 0; i < 6; ++i) {
                    auto neighbor = randomCandidate.nodeInDir(i);
                    if (considered.find(neighbor) == considered.end()) candidates.insert(neighbor);

                }
            }
        }
    }

    // Place particles
    // First, place leader particle

    Node maxNode = Node(0,0);
    for(auto &tile : tiles) {
        if (maxNode < tile->node) maxNode = tile->node;
    }

    leader = new ConvexHullParticle(Node(maxNode.x, maxNode.y + 1), -1, randDir(), *this,
                                ConvexHullParticle::State::Leader);
    insert(leader);

    // Place other particles
    candidates.clear();
    Node leaderPos = leader->head;
    for (int i = 0; i < 6; ++i) {
        if (tilePositions.find(leaderPos.nodeInDir(i)) == tilePositions.end()) candidates.insert(leaderPos.nodeInDir(i));
    }

    std::set<Node> particlePositions; //contains all nodes occupied by particles
    particlePositions.insert(leaderPos);

    while (particlePositions.size() < numParticles) {
        // Pick random candidate.
        std::set<Node>::const_iterator it(candidates.begin());
        advance(it,randInt(0,candidates.size()));
        Node randomCandidate = *it;
        candidates.erase(it);

        ConvexHullParticle* particle = new ConvexHullParticle(randomCandidate, -1, randDir(), *this,
                                                             ConvexHullParticle::State::Idle);
        insert(particle);
        particlePositions.insert(randomCandidate);

        // Add new candidates.
        for (int i = 0; i < 6; ++i) {
            auto neighbor = randomCandidate.nodeInDir(i);
            if (particlePositions.find(randomCandidate.nodeInDir(i)) == particlePositions.end() && tilePositions.find(randomCandidate.nodeInDir(i)) == tilePositions.end()) candidates.insert(neighbor);

        }
    }

}

std::vector<Node> ConvexHullSystem::getConvexHullApproximate() const {
    std::vector<int> offset({1, 0, -1, -1, 0, 1});
    std::vector<Node> convexVertices(6);
    std::vector<int> distance(6);
    int orientOffset = 6 - leader->orientation;
    for(int i = 0; i < 6; i++) {
        distance[i] = leader->distance[(i + orientOffset) % 6];
    }

    for(int i = 0; i<6; i++) {
        int x = leader->head.x;
        int y = leader->head.y;

        convexVertices[i] = Node(x + (offset[i] * distance[((i + 5) % 6)]) + (offset[((i + 1) % 6)] * (distance[i] - distance[((i + 5) % 6)])), y + (offset[((i + 4) % 6)] * distance[((i + 5) % 6)]) + (offset[((i + 5) % 6)] * (distance[i] - distance[((i + 5) % 6)])));
    }

    return convexVertices;
}

bool ConvexHullSystem::hasTerminated() const {

  for (auto p : particles) {
    auto hp = dynamic_cast<ConvexHullParticle*>(p);
    if (hp->state != ConvexHullParticle::State::FollowerDone && hp->state != ConvexHullParticle::State::LeaderDone) {
      return false;
    }
  }

  return true;
}
