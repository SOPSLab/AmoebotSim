/* Copyright (C) 2019 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/demo/metricsdemo.h"

MetricsDemoParticle::MetricsDemoParticle(const Node head, const int globalTailDir,
                                     const int orientation,
                                     AmoebotSystem& system,
                                     const int counterMax)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      _counter(counterMax),
      _counterMax(counterMax) {
  _state = getRandColor();
}

void MetricsDemoParticle::activate() {
  // First decrement the particle's counter. If it's zero, reset the counter and
  // get a new color.
  _counter--;
  if (_counter == 0) {
    _counter = _counterMax;
    _state = getRandColor();
  }

  // Next, handle movement. If the particle is contracted, choose a random
  // direction to try to expand towards, but only do so if the node in that
  // direction is unoccupied. Otherwise, if the particle is expanded, simply
  // contract its tail.
  if (isContracted()) {
    int expandDir = randDir();
    if (canExpand(expandDir)) {
      expand(expandDir);
    } else {
      if (hasObjectAtLabel(expandDir)) {
        system.getCount("# Bumps Into Wall").record();
      }
    }
  } else {  // isExpanded().
    contractTail();
  }
}

int MetricsDemoParticle::headMarkColor() const {
  switch(_state) {
    case State::Red:    return 0xff0000;
    case State::Orange: return 0xff9000;
    case State::Yellow: return 0xffff00;
    case State::Green:  return 0x00ff00;
    case State::Blue:   return 0x0000ff;
    case State::Indigo: return 0x4b0082;
    case State::Violet: return 0xbb00ff;
  }

  return -1;
}

int MetricsDemoParticle::tailMarkColor() const {
  return headMarkColor();
}

QString MetricsDemoParticle::inspectionText() const {
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
                      + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Local Info:\n";
  text += "  state: ";
  text += [this](){
    switch(_state) {
      case State::Red:    return "red\n";
      case State::Orange: return "orange\n";
      case State::Yellow: return "yellow\n";
      case State::Green:  return "green\n";
      case State::Blue:   return "blue\n";
      case State::Indigo: return "indigo\n";
      case State::Violet: return "violet\n";
    }
    return "no state\n";
  }();
  text += "  counter: " + QString::number(_counter);

  return text;
}

MetricsDemoParticle::State MetricsDemoParticle::getRandColor() const {
  // Randomly select an integer and return the corresponding state via casting.
  return static_cast<State>(randInt(0, 7));
}

MetricsDemoSystem::MetricsDemoSystem(unsigned int numParticles, int counterMax) {
  _counts.push_back(new Count("# Bumps Into Wall"));
  _measures.push_back(new PercentageRedMeasure("Percentage Red", 1, *this));
  _measures.push_back(new MaxDistanceMeasure("Max 2 Particle Dist", 1, *this));

  // In order to enclose an area that's roughly 3.7x the # of particles using a
  // regular hexagon, the hexagon should have side length 1.4*sqrt(# particles).
  int sideLen = static_cast<int>(std::round(1.4 * std::sqrt(numParticles)));
  Node boundNode(0, 0);
  for (int dir = 0; dir < 6; ++dir) {
    for (int i = 0; i < sideLen; ++i) {
      insert(new Object(boundNode));
      boundNode = boundNode.nodeInDir(dir);
    }
  }

  // Let s be the bounding hexagon side length. When the hexagon is created as
  // above, the nodes (x,y) strictly within the hexagon have (i) -s < x < s,
  // (ii) 0 < y < 2s, and (iii) 0 < x+y < 2s. Choose interior nodes at random to
  // place particles, ensuring at most one particle is placed at each node.
  std::set<Node> occupied;
  while (occupied.size() < numParticles) {
    // First, choose an x and y position at random from the (i) and (ii) bounds.
    int x = randInt(-sideLen + 1, sideLen);
    int y = randInt(1, 2 * sideLen);
    Node node(x, y);

    // If the node satisfies (iii) and is unoccupied, place a particle there.
    if (0 < x + y && x + y < 2 * sideLen
        && occupied.find(node) == occupied.end()) {
      insert(new MetricsDemoParticle(node, -1, randDir(), *this, counterMax));
      occupied.insert(node);
    }
  }
}

PercentageRedMeasure::PercentageRedMeasure(const QString name, const unsigned int freq,
                                           MetricsDemoSystem& system)
  : Measure(name, freq),
    _system(system) {}

double PercentageRedMeasure::calculate() const {
  int numRed = 0;

  for (const auto& p : _system.particles) {
    auto metr_p = dynamic_cast<MetricsDemoParticle*>(p);
    if (metr_p->_state == MetricsDemoParticle::State::Red) {
      numRed++;
    }
  }

  return ( (double(numRed) / double(_system.size())) * 100);
}

MaxDistanceMeasure::MaxDistanceMeasure(const QString name, const unsigned int freq,
                                           MetricsDemoSystem& system)
  : Measure(name, freq),
    _system(system) {}

double MaxDistanceMeasure::calculate() const {
  double dist;
  double maxDist = 0.0;

  for (const auto& p1 : _system.particles) {
    double x1_c = p1->head.x + p1->head.y/2.0;
    double y1_c = sqrt(3.0)/2 * p1->head.y;
    for (const auto& p2 : _system.particles) {
      double x2_c = p2->head.x + p2->head.y/2.0;
      double y2_c = sqrt(3.0)/2 * p2->head.y;
      dist = sqrt( pow((x2_c - x1_c), 2) + pow((y2_c - y1_c), 2) );
      if (dist > maxDist) {
        maxDist = dist;
      }
    }
  }

  return maxDist;
}
