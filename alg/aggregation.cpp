#include <QDebug>

#include "aggregation.h"

AggregateParticle::AggregateParticle(const Node head, const int globalTailDir,
                                     const int orientation,
                                     AmoebotSystem& system,
                                     int center,
                                     std::vector<AggregateParticle*> particles)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    center(center),
    particles(particles) {}

void AggregateParticle::activate() {
  bool particleInSight = checkIfParticleInSight();
  // int probNum = 0;


// // Original algorithm but with added randomness/probabilities
//  if (particleInSight) {
//    probNum = randInt(0, 1000);
//    if (probNum < 1000) {
//      center = (center + 5) % 6;
//    } else {
//      int moveDir = (center + 1) % 6;
//      if (!hasNbrAtLabel(moveDir)) {
//        expand(moveDir);
//        contractTail();
//        center = (center + 5) % 6;
//      }
//    }
//  }
//  else {
//    probNum = randInt(0, 1000);
//    if (probNum < 985){
//      int moveDir = (center + 1) % 6;
//      if (!hasNbrAtLabel(moveDir)) {
//        expand(moveDir);
//        contractTail();
//        center = (center + 5) % 6;
//      }
//    } else {
//      center = (center + 5) % 6;
//    }
//  }


// // Original algorithm but with perturb variable
//  if (particleInSight) {
//    center = (center + 5) % 6;
//    perturb = 6;
//  }
//  else if (perturb <= 0) {
//    probNum = randInt(0, 1000);
//    if (probNum < 985) {
//      center = (center + 5) % 6;
//    }
//  }
//  else {
//    int moveDir = (center + 1) % 6;
//    if (!hasNbrAtLabel(moveDir)) {
//      expand(moveDir);
//      contractTail();
//      center = (center + 5) % 6;
//    }
//    perturb--;
//  }


// // Perturb method but with circular, sweeping motion
//  if (particleInSight) {
//    center = (center + 5) % 6;
//    perturb = 6;
//  }
//  else if (perturb <= 0) {
//    center = (center + 5) % 6;
//  }
//  else {
//   int moveDir = (center + 3) % 6;
//   if (!hasNbrAtLabel(moveDir)) {
//     expand(moveDir);
//     contractTail();
//     center = (center + 5) % 6;
//   }
//   perturb--;
//  }


// Perturb method but everything counter-clockwise instead of clockwise
  if (particleInSight) {
    center = (center + 1) % 6;
    perturb = 6;
  }
  else if (perturb <= 0) {
     center = (center + 1) % 6;
  }
  else {
   int moveDir = (center + 1) % 6;
   if (!hasNbrAtLabel(moveDir)) {
     expand(moveDir);
     contractTail();
     center = (center + 1) % 6;
   }
   perturb--;
  }


}

int AggregateParticle::headMarkColor() const {
  return 0xffff00;
}

int AggregateParticle::tailMarkColor() const {
  return headMarkColor();
}

int AggregateParticle::headMarkDir() const {
  return (center + 5) % 6;
}

QString AggregateParticle::inspectionText() const {
  QString text;
  text += "head: (" + QString::number(head.x) + ", " + QString::number(head.y) +
    ")\n";
  text += "orientation: " + QString::number(orientation) + "\n";
  text += "globalTailDir: " + QString::number(globalTailDir) + "\n";
  text += "\n";
  text += "center: " + QString::number(center) + "\n";
  text += "sight: " + QString::number((center + 5) % 6) + "\n";
  text += "particle in sight: " + QString::number(this->checkIfParticleInSight()) + "\n";
  return text;
}

AggregateParticle& AggregateParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<AggregateParticle>(label);
}

bool AggregateParticle::checkIfParticleInSight() const {
  int sightScope = (center + 4) % 6;

  for (auto p: particles) {
    int distX = abs(p->head.x - head.x);
    int distY = abs(p->head.y - head.y);

    switch (sightScope) {
      case 0:
        if (p->head.y > head.y && p->head.x >= head.x) {
          return true;
        }
        break;
      case 1:
        if (p->head.y > head.y && p->head.x < head.x && distX >= distY) {
          return true;
        }
        break;
      case 2:
        if (p->head.y >= head.y && p->head.x < head.x && distY < distX) {
          return true;
        }
        break;
      case 3:
        if (p->head.y < head.y && p->head.x <= head.x) {
          return true;
        }
        break;
      case 4:
        if (p->head.y < head.y && p->head.x > head.x && distY >= distX) {
          return true;
        }
        break;
      case 5:
        if (p->head.y <= head.y && p->head.x > head.x && distY < distX) {
          return true;
        }
        break;
      default:
        Q_ASSERT(center >= 0 && center < 6);
    }
  }
  return false;
}

AggregateSystem::AggregateSystem(int numParticles) {
  _measures.push_back(new MaxDistanceMeasure("MAX 2 Particle Dist", 1, *this));
  _measures.push_back(new SumDistancesMeasure("SUM 2 Particle Dists", 1, *this));

  Q_ASSERT(numParticles > 0);
  std::set<Node> occupied;
  std::vector<AggregateParticle*> particles;

  long boxRadius = lround(numParticles * 0.25);
  if (numParticles < 50) {
    boxRadius = lround(50 * 0.25);
  }

  int n = 0;
  while (n < numParticles) {
    int x = randInt(-1 * boxRadius, boxRadius);
    int y = randInt(-1 * boxRadius, boxRadius);
    if (occupied.find(Node(x, y)) == occupied.end()) {
      AggregateParticle* particle =
          new AggregateParticle(Node(x, y), -1, 0, *this, randDir(), particles);
      insert(particle);
      particles.insert(particles.end(), particle);
      occupied.insert(Node(x, y));
      ++n;
    }
  }

  for (auto p : particles) {
    p->particles = particles;
  }
}

MaxDistanceMeasure::MaxDistanceMeasure(const QString name, const unsigned int freq,
                                           AggregateSystem& system)
  : Measure(name, freq),
    _system(system) {}

double MaxDistanceMeasure::calculate() const {
  double dist;
  double maxDist = 0.0;
  int x1_center, y1_center, x2_center, y2_center;

  for (const auto& p1 : _system.particles) {
    auto aggr_p1 = dynamic_cast<AggregateParticle*>(p1);

    switch (aggr_p1->center) {
      case 0 :
        x1_center = aggr_p1->head.x + 1;
        y1_center = aggr_p1->head.y;
        break;
      case 1 :
        x1_center = aggr_p1->head.x;
        y1_center = aggr_p1->head.y + 1;
        break;
      case 2 :
        x1_center = aggr_p1->head.x - 1;
        y1_center = aggr_p1->head.y + 1;
        break;
      case 3 :
        x1_center = aggr_p1->head.x - 1;
        y1_center = aggr_p1->head.y;
        break;
      case 4 :
        x1_center = aggr_p1->head.x;
        y1_center = aggr_p1->head.y - 1;
        break;
      case 5 :
        x1_center = aggr_p1->head.x + 1;
        y1_center = aggr_p1->head.y - 1;
        break;
    }

    double x1_car = x1_center + y1_center/2.0;
    double y1_car = sqrt(3.0)/2 * y1_center;

    for (const auto& p2 : _system.particles) {
      auto aggr_p2 = dynamic_cast<AggregateParticle*>(p2);

      switch (aggr_p2->center) {
        case 0 :
          x2_center = aggr_p2->head.x + 1;
          y2_center = aggr_p2->head.y;
          break;
        case 1 :
          x2_center = aggr_p2->head.x;
          y2_center = aggr_p2->head.y + 1;
          break;
        case 2 :
          x2_center = aggr_p2->head.x - 1;
          y2_center = aggr_p2->head.y + 1;
          break;
        case 3 :
          x2_center = aggr_p2->head.x - 1;
          y2_center = aggr_p2->head.y;
          break;
        case 4 :
          x2_center = aggr_p2->head.x;
          y2_center = aggr_p2->head.y - 1;
          break;
        case 5 :
          x2_center = aggr_p2->head.x + 1;
          y2_center = aggr_p2->head.y - 1;
          break;
      }

      double x2_car = x2_center + y2_center/2.0;
      double y2_car = sqrt(3.0)/2 * y2_center;

      dist = sqrt( pow((x2_car - x1_car), 2) + pow((y2_car - y1_car), 2) );
      if (dist > maxDist) {
        maxDist = dist;
      }
    }
  }

  return maxDist;
}

SumDistancesMeasure::SumDistancesMeasure(const QString name, const unsigned int freq,
                                           AggregateSystem& system)
  : Measure(name, freq),
    _system(system) {}

double SumDistancesMeasure::calculate() const {
  double dist;
  double totalDist = 0.0;
  int x1_center, y1_center, x2_center, y2_center;

  for (const auto& p1 : _system.particles) {
    auto aggr_p1 = dynamic_cast<AggregateParticle*>(p1);

    switch (aggr_p1->center) {
      case 0 :
        x1_center = aggr_p1->head.x + 1;
        y1_center = aggr_p1->head.y;
        break;
      case 1 :
        x1_center = aggr_p1->head.x;
        y1_center = aggr_p1->head.y + 1;
        break;
      case 2 :
        x1_center = aggr_p1->head.x - 1;
        y1_center = aggr_p1->head.y + 1;
        break;
      case 3 :
        x1_center = aggr_p1->head.x - 1;
        y1_center = aggr_p1->head.y;
        break;
      case 4 :
        x1_center = aggr_p1->head.x;
        y1_center = aggr_p1->head.y - 1;
        break;
      case 5 :
        x1_center = aggr_p1->head.x + 1;
        y1_center = aggr_p1->head.y - 1;
        break;
    }

    double x1_car = x1_center + y1_center/2.0;
    double y1_car = sqrt(3.0)/2 * y1_center;

    for (const auto& p2 : _system.particles) {
      auto aggr_p2 = dynamic_cast<AggregateParticle*>(p2);

      switch (aggr_p2->center) {
        case 0 :
          x2_center = aggr_p2->head.x + 1;
          y2_center = aggr_p2->head.y;
          break;
        case 1 :
          x2_center = aggr_p2->head.x;
          y2_center = aggr_p2->head.y + 1;
          break;
        case 2 :
          x2_center = aggr_p2->head.x - 1;
          y2_center = aggr_p2->head.y + 1;
          break;
        case 3 :
          x2_center = aggr_p2->head.x - 1;
          y2_center = aggr_p2->head.y;
          break;
        case 4 :
          x2_center = aggr_p2->head.x;
          y2_center = aggr_p2->head.y - 1;
          break;
        case 5 :
          x2_center = aggr_p2->head.x + 1;
          y2_center = aggr_p2->head.y - 1;
          break;
      }

      double x2_car = x2_center + y2_center/2.0;
      double y2_car = sqrt(3.0)/2 * y2_center;

      dist = sqrt( pow((x2_car - x1_car), 2) + pow((y2_car - y1_car), 2) );
      totalDist = totalDist + dist;
    }
  }

  return totalDist;
}

bool AggregateSystem::hasTerminated() const {
  return false;
}
