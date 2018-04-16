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
    turns(0),
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
                if (!onHull && preHull) {
                    onHull = true;
                    preHull = false;
                }
            }
//            else {
//                pullChildIfPossible();
//            }
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

                        // Update completed vector
                        if (distance[moveDir] == 0 or distance[((moveDir + 5) % 6)] == 0) std::fill(completed.begin(), completed.end(), 0);
                        else {
                            for (int i = 0; i < 6; i++) {
                                if (distance[i] + delta[moveDir][i] == 0) completed[i] = 1;
                            }
                        }
                        break;
                    }
                }
            }

            else {
                // Phase 2: Follow the convex hull

                if (!onHull) preHull = true;

                if (distance[moveDir] == 0) {
                    moveDir = (moveDir + 5) % 6;
                    turns++;
                }

                // Check if already completed Phase 2
                if (turns == 14 && hasTileAtLabel((moveDir+5) % 6)) {
                    state = State::LeaderWait;
                    return;
                }

                // Wait on hull particles
                ConvexHullParticle& nbr = nbrAtLabel<ConvexHullParticle>(moveDir);
                if (nbr.state == State::Follower && nbr.onHull == true) {
                    return;
                }
            }

            // Perform the move

            // Update distances
            for(int i = 0; i < 6; i++) distance[i] = std::max(0, distance[i] + delta[moveDir][i]);

            // Check if there is a neighbor in moveDir (can be a non-hull Follower or an Idle Particle)
            if(hasNbrAtLabel(moveDir)) {
                if (canPush(moveDir)) {
                    if (hasHeadAtLabel(moveDir)) {
                        ConvexHullParticle& p = nbrAtLabel<ConvexHullParticle>(moveDir);
                        p.parentDir = (p.tailDir() + 3) % 6;
                    }
                    push(moveDir);
                }
                else {
                    // Swap leader role
                    swapWithFollowerInDir(moveDir);
                }
            }
            else {
                expand(moveDir);
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
                if (!onHull && preHull) {
                    onHull = true;
                    preHull = false;
                }
            }
//            else {
//                pullChildIfPossible();
//            }
        }
        else {
            if (neighborInDirIsInState(parentDir, {State::LeaderWait, State::FollowerWait})) {
                state = State::FollowerWait;
            }
            else {
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

                if (hasNbrAtLabel((moveDir+3) % 6) and neighborInDirIsInState((moveDir+3) % 6, {State::FollowerWait2})) {
                    predecessor = (moveDir+3) % 6;
                }
                else {
                    Q_ASSERT(hasNbrAtLabel((moveDir + 4) % 6));
                    Q_ASSERT(neighborInDirIsInState((moveDir+4) % 6, {State::FollowerWait2}));
                    predecessor = (moveDir+4) % 6;
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
        if (neighborInDirIsInState(parentDir, {State::LeaderDone, State::FollowerDone})) {
            state = State::FollowerDone;
        }
        else if (neighborInDirIsInState(parentDir, {State::LeaderOrtho, State::FollowerOrtho})) {

            if (onHull) {
                // Set successor and predecessor pointers
                successor = parentDir;
                if (hasNbrAtLabel((parentDir+3) % 6) and neighborInDirIsInState((parentDir+3) % 6, {State::FollowerWait2})) {
                    predecessor = (parentDir+3) % 6;
                }
                else {
                    Q_ASSERT(hasNbrAtLabel((parentDir + 4) % 6));
                    Q_ASSERT(neighborInDirIsInState((parentDir + 4) % 6, {State::FollowerWait2}));
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

bool ConvexHullParticle::hasNeighborInState(std::initializer_list<State> states) const {
  return labelOfFirstNbrInState(states) != -1;
}

bool ConvexHullParticle::neighborInDirIsInState(int dir, std::initializer_list<State> states) const {
    Q_ASSERT(0 <= dir && dir < 6);
    Q_ASSERT(hasNbrAtLabel(dir));

    const ConvexHullParticle& p = nbrAtLabel<ConvexHullParticle>(dir);
    for (auto state : states) {
      if (p.state == state) return true;
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
    Q_ASSERT(neighborInDirIsInState(dir, {State::Idle, State::Follower}));

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

    nbr.turns = turns;
    nbr.onHull = onHull;

    // Update self
    state = State::Follower;
    moveDir = 0;
    parentDir = dir;
    std::fill(distance.begin(), distance.end(), 0);
    std::fill(completed.begin(), completed.end(), 0);
    turns = 0;

    // Update pointer to leader (for visualization)
    dynamic_cast<ConvexHullSystem&>(system).leader = &nbr;
}

bool ConvexHullParticle::hasChild() const {
    auto propertyCheck = [&](const ConvexHullParticle& p) {
      return p.state == State::Follower &&
             ((isContracted() && pointsAtMe(p, p.dirToHeadLabel(p.parentDir))) or
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
    // Warning: Pull does not seem to work properly!
    auto propertyCheck = [&](const ConvexHullParticle& p) {
      return p.state == State::Follower &&
             pointsAtMyTail(p, p.dirToHeadLabel(p.parentDir)) &&
              p.isContracted();
    };

    int dir = labelOfFirstNbrWithProperty<ConvexHullParticle>(propertyCheck);

    if (dir != -1) {
        ConvexHullParticle& p = nbrAtLabel<ConvexHullParticle>(dir);
        int contractionDir = dirToNbrDir(p, (tailDir() + 3) % 6);
        pull(dir);
        p.parentDir = contractionDir;
    }
}

void ConvexHullParticle::pushParentIfPossible() {
    if (hasTailAtLabel(parentDir)) {
        // If on the hull already, give preference to particles inside the hull to enter
        if (onHull && hasNbrAtLabel((parentDir + 5) % 6)) {
            ConvexHullParticle& nbr = nbrAtLabel<ConvexHullParticle>((parentDir + 5) % 6);
            if (nbrDirToDir(nbr, nbr.parentDir) == ((parentDir + 1) % 6)) return;
        }

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
  else if (state == State::Follower) {
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
