#include <cmath>
#include <QDebug>
#include <set>

#include "swarmseparation.h"

SwarmSeparationParticle::SwarmSeparationParticle(const Node head,
                                               const int globalTailDir,
                                               const int orientation,
                                               AmoebotSystem& system,
                                               State state,
                                               double randConstant)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    randConstant(randConstant) {}

void SwarmSeparationParticle::activate() {
  int expandDir = vectorCalculation();

  if (expandDir != -1) {
    if (canExpand(expandDir)) {
      expand(expandDir);
      contractTail();
    }
  }
}

int SwarmSeparationParticle::headMarkColor() const {
  switch(state) {
    case State::Small:  return 0xff0000;
    case State::Medium: return 0x00ff00;
    case State::Large:  return 0x0000ff;
  }

  return -1;
}

int SwarmSeparationParticle::tailMarkColor() const {
  return headMarkColor();
}

QString SwarmSeparationParticle::inspectionText() const {
  QString text;
  text += "head: (" + QString::number(head.x) + ", " + QString::number(head.y) +
    ")\n";
  text += "orientation: " + QString::number(orientation) + "\n";
  text += "globalTailDir: " + QString::number(globalTailDir) + "\n";
  text += "state: ";
  switch(state) {
    case State::Small:  text += "small"; break;
    case State::Medium: text += "medium"; break;
    case State::Large:  text += "large"; break;
    Q_ASSERT(false);
  }
  text += "\n";
  text += "randConstant: " + QString::number(randConstant) + "\n";
  text += "\n";

  return text;
}

int SwarmSeparationParticle::vectorCalculation() {
  // Built a taxis vector in square coordinates based on hexagonal coordinates.
  double taxis_x = -1 * (head.x + head.y / 2.0);
  double taxis_y = -1 * (sqrt(3) * head.y / 2.0);

  // Choose a random vector in [0, 2pi).
  double rand = randDouble(0, 2 * M_PI);
  double rand_x = cos(rand);
  double rand_y = sin(rand);

  // Combine the vectors in normalized form.
  double result_x = (taxis_x / vectorMag(taxis_x, taxis_y))
                    + randConstant * (rand_x / vectorMag(rand_x, rand_y));
  double result_y = (taxis_y / vectorMag(taxis_x, taxis_y))
                    + randConstant * (rand_y / vectorMag(rand_x, rand_y));

  // Only move if the magnitude of the proposed movement is above a threshold.
  double threshold = 0;
  if (vectorMag(result_x, result_y) > threshold) {
    // Get the angle of the move in degrees, transformed to hexagonal coords.
    double angle = atan(result_y / result_x) * 180.0 / M_PI;
    if (result_x < 0) {
      angle += 180;
    } else if (result_x >= 0 && -90 <= angle && angle < 0) {
      angle += 360;
    }

    // Map angles to movement directions.
    if (30 <= angle && angle < 90) {
      return 1;
    } else if (90 <= angle && angle < 150) {
      return 2;
    } else if (150 <= angle && angle < 210) {
      return 3;
    } else if (210 <= angle && angle < 270) {
      return 4;
    } else if (270 <= angle && angle < 330) {
      return 5;
    } else {
      return 0;
    }
  }

  return -1;
}

double SwarmSeparationParticle::vectorMag(const double x, const double y) {
  return sqrt(pow(x, 2) + pow(y, 2));
}

SwarmSeparationSystem::SwarmSeparationSystem(int numParticles,
                                             double randConstant) {
  Q_ASSERT(numParticles > 0);
  std::set<Node> occupied;

  int boxradius = lround(numParticles * 0.25);
  int n = 0;

  while (n < numParticles) {
    int x = randInt(-1 * boxradius, boxradius);
    int y = randInt(-1 * boxradius, boxradius);
    if (occupied.find(Node(x,y)) == occupied.end()) {
      insert(new SwarmSeparationParticle(Node(x,y), -1, 0, *this,
                 SwarmSeparationParticle::State(randInt(0, 3)), randConstant));
      occupied.insert(Node(x,y));
      ++n;
    }
  }
}

bool SwarmSeparationSystem::hasTerminated() const {
	return false;
}


