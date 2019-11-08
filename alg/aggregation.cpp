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
  int probNum = 0;

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

bool AggregateSystem::hasTerminated() const {
  return false;
}
