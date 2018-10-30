#include <cmath>
#include <set>

#include "swarmseparation.h"

SwarmSeparationParticle::SwarmSeparationParticle(const Node head,
                                                 const int globalTailDir,
                                                 const int orientation,
                                                 AmoebotSystem& system,
                                                 State state, double c_rand,
                                                 double c_repulse)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    state(state),
    c_rand(c_rand),
    c_repulse(c_repulse) {}

void SwarmSeparationParticle::activate() {
  std::pair<double, double> moveVector = getMoveVector();

  // Only move if the magnitude of the proposed movement is above a threshold.
  int expandDir = -1;
  double threshold = 0;
  if (magnitude(moveVector) > threshold) {
    // Get the angle of the move in degrees.
    double angle = atan2(moveVector.second, moveVector.first) * 180.0 / M_PI;
    if (angle < 0) {
      angle += 360;
    }

    // Map angles to movement directions on the triangular lattice.
    if (30 <= angle && angle < 90) {
      expandDir = 1;
    } else if (90 <= angle && angle < 150) {
      expandDir = 2;
    } else if (150 <= angle && angle < 210) {
      expandDir = 3;
    } else if (210 <= angle && angle < 270) {
      expandDir = 4;
    } else if (270 <= angle && angle < 330) {
      expandDir = 5;
    } else {
      expandDir = 0;
    }
  }

  if (expandDir != -1 && canExpand(expandDir)) {
    expand(expandDir);
    contractTail();
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
  text += "c_rand: " + QString::number(c_rand) + "\n";
  text += "c_repulse: " + QString::number(c_repulse) + "\n";
  text += "\n";

  return text;
}

std::pair<double, double> SwarmSeparationParticle::getTaxisVector() {
  // The light source is at (0,0), so -<head.x, head.y> points at the light.
  auto taxis = std::make_pair<double, double> (-1 * xTriToCart(head.x, head.y),
                                               -1 * yTriToCart(head.y));
  normalize(taxis);

  return taxis;
}

std::pair<double, double> SwarmSeparationParticle::getRandVector() {
  double randAngle = randDouble(0, 2 * M_PI);
  auto rand = std::make_pair<double, double> (cos(randAngle), sin(randAngle));
  normalize(rand);

  return rand;
}

std::pair<double, double> SwarmSeparationParticle::getRepulseVector() {
  int radius = 0;
  if (state == State::Small) {
    radius = 1;
  } else if (state == State::Medium) {
    radius = 4;
  } else if (state == State::Large) {
    radius = 8;
  }

  auto repulse = std::make_pair<double, double> (0, 0);
  for (auto p : system.particles) {
    if (p->head != head && nodeDist(p->head, head) < 2 * radius - 1) {
      double x_diff = xTriToCart(head.x - p->head.x, head.y - p->head.y);
      double y_diff = yTriToCart(head.y - p->head.y);
      repulse.first += cos(atan2(y_diff, x_diff));
      repulse.second += sin(atan2(y_diff, x_diff));
    }
  }
  normalize(repulse);

  return repulse;
}

std::pair<double, double> SwarmSeparationParticle::getMoveVector() {
  auto taxis = getTaxisVector();
  auto rand = getRandVector();
  auto repulse = getRepulseVector();

  repulse.first = c_repulse * repulse.first;
  repulse.second = c_repulse * repulse.second;
  normalize(repulse);

  return std::make_pair<double, double>
      (taxis.first + (c_rand * rand.first) + repulse.first,
       taxis.second + (c_rand * rand.second) + repulse.second);
}

double SwarmSeparationParticle::magnitude(const std::pair<double, double> &vec) {
  return sqrt(pow(vec.first, 2) + pow(vec.second, 2));
}

void SwarmSeparationParticle::normalize(std::pair<double, double> &vec) {
  if (std::abs(vec.first) > std::numeric_limits<double>::epsilon() ||
      std::abs(vec.second) > std::numeric_limits<double>::epsilon()) {
    double mag = magnitude(vec);
    vec.first = vec.first / mag;
    vec.second = vec.second / mag;
  }
}

double SwarmSeparationParticle::xTriToCart(const double x, const double y) {
  return x + y / 2.0;
}

double SwarmSeparationParticle::yTriToCart(const double y) {
  return sqrt(3) * y / 2.0;
}

double SwarmSeparationParticle::nodeDist(const Node &n1, const Node &n2) {
  auto n1Cart = std::make_pair<double, double> (xTriToCart(n1.x, n1.y),
                                                yTriToCart(n1.y));
  auto n2Cart = std::make_pair<double, double> (xTriToCart(n2.x, n2.y),
                                                yTriToCart(n2.y));
  auto diff = std::make_pair<double, double> (n1Cart.first - n2Cart.first,
                                              n1Cart.second - n2Cart.second);

  return magnitude(diff);
}

SwarmSeparationSystem::SwarmSeparationSystem(int numParticles, double c_rand,
                                             double c_repulse) {
  Q_ASSERT(numParticles > 0);
  std::set<Node> occupied;

  int boxradius = lround(numParticles * 0.25);
  int n = 0;

  while (n < numParticles) {
    int x = randInt(-1 * boxradius, boxradius);
    int y = randInt(-1 * boxradius, boxradius);
    if (occupied.find(Node(x,y)) == occupied.end()) {
      insert(new SwarmSeparationParticle(Node(x,y), -1, 0, *this,
             SwarmSeparationParticle::State(randInt(0, 3)), c_rand, c_repulse));
      occupied.insert(Node(x,y));
      ++n;
    }
  }
}

bool SwarmSeparationSystem::hasTerminated() const {
	return false;
}


