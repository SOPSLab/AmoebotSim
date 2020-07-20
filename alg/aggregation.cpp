#include <QDebug>

#include <math.h>
#include <unordered_set>

#include "aggregation.h"

using namespace std;

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




//  // Error probability
//  double probNum = randDouble(0, 1);
//  if (probNum < errorProb) {
//    if (particleInSight == true) {
//      particleInSight = false;
//    }
//    else {
//      particleInSight = true;
//    }
//  }

//  if (particleInSight) {
//    center = (center + 5) % 6;
//  }
//  else {
//   int moveDir = (center + 1) % 6;
//   if (!hasNbrAtLabel(moveDir)) {
//     expand(moveDir);
//     contractTail();
//     center = (center + 5) % 6;
//   }
//  }




  // Perturbation
  if (particleInSight) {
    center = (center + 5) % 6;
//    perturb = perturbResetVal;
    perturb = 4;
  }
  else {
   int moveDir = (center + 1) % 6;
   if (!hasNbrAtLabel(moveDir)) {
     expand(moveDir);
     contractTail();
     center = (center + 5) % 6;
   } else {
     perturb--;
     if (perturb <= 0) {
      center = (center + 5) % 6;
//      perturb = perturbResetVal;
      perturb = 4;
     }
   }
  }




//  // Different rates for rotate in place vs. move around center of rotation
//  // 150->.15, 1/.15 = 6.6667, rotating in place happens 6.667 times as often
//  // (as "fast") as rotating around center of rotation
//  int rateDiff = 150;
//  if (particleInSight) {
//    center = (center + 5) % 6;
//  }
//  else {
//    int randNum = randInt(0, 1000);
//    if (randNum >= (1000 - rateDiff)) {
//      int moveDir = (center + 1) % 6;
//      if (!hasNbrAtLabel(moveDir)) {
//        expand(moveDir);
//        contractTail();
//        center = (center + 5) % 6;
//      }
//    }
//  }




//  // different rates mixed with perturbation
//  int rateDiff = 150;
//  if (particleInSight) {
//    center = (center + 5) % 6;
//    perturb = 4;
//  }
//  else {
//    int randNum = randInt(0, 1000);
//    if (randNum >= (1000 - rateDiff)) {
//      int moveDir = (center + 1) % 6;
//      if (!hasNbrAtLabel(moveDir)) {
//        expand(moveDir);
//        contractTail();
//        center = (center + 5) % 6;
//      } else {
//        perturb--;
//        if (perturb <= 0) {
//          center = (center + 5) % 6;
//          perturb = 4;
//        }
//      }
//    }
//  }

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
  int sightScope = (center + 4) % 6;    // MAIN; 90 degree angle from center
//  int sightScope = (center + 2) % 6;    // optimized for counterclockwise
//  int sightScope = (center + 3) % 6;    // optimized for clockwise (and for ray)


  // FIXED: cone from sightscope variable val to sightScope+1; sightscope val dashed, sightscope+1 included
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
        if (p->head.y > head.y && p->head.x < head.x && distY >= distX) {
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


// // old, incorrect: cone from sightscope variable val to sightScope+1; sightscope val dashed, sightscope+1 included
//  for (auto p: particles) {
//    int distX = abs(p->head.x - head.x);
//    int distY = abs(p->head.y - head.y);

//    switch (sightScope) {
//      case 0:
//        if (p->head.y > head.y && p->head.x >= head.x) {
//          return true;
//        }
//        break;
//      case 1:
//        if (p->head.y > head.y && p->head.x < head.x && distX >= distY) {
//          return true;
//        }
//        break;
//      case 2:
//        if (p->head.y >= head.y && p->head.x < head.x && distY < distX) {
//          return true;
//        }
//        break;
//      case 3:
//        if (p->head.y < head.y && p->head.x <= head.x) {
//          return true;
//        }
//        break;
//      case 4:
//        if (p->head.y < head.y && p->head.x > head.x && distY >= distX) {
//          return true;
//        }
//        break;
//      case 5:
//        if (p->head.y <= head.y && p->head.x > head.x && distY < distX) {
//          return true;
//        }
//        break;
//      default:
//        Q_ASSERT(center >= 0 && center < 6);
//    }
//  }
//  return false;


//  // cone from sightscope variable val to sightScope+1; sightscope val included, sightscope+1 dashed line
//  for (auto p: particles) {
//    int distX = abs(p->head.x - head.x);
//    int distY = abs(p->head.y - head.y);

//    switch (sightScope) {
//      case 0:
//        if (p->head.y >= head.y && p->head.x > head.x) {
//          return true;
//        }
//        break;
//      case 1:
//        if (p->head.y > head.y && p->head.x <= head.x && distY > distX) {
//          return true;
//        }
//        break;
//      case 2:
//        if (p->head.y > head.y && p->head.x < head.x && distY <= distX) {
//          return true;
//        }
//        break;
//      case 3:
//        if (p->head.y <= head.y && p->head.x < head.x) {
//          return true;
//        }
//        break;
//      case 4:
//        if (p->head.y < head.y && p->head.x >= head.x && distY > distX) {
//          return true;
//        }
//        break;
//      case 5:
//        if (p->head.y < head.y && p->head.x > head.x && distY <= distX) {
//          return true;
//        }
//        break;
//      default:
//        Q_ASSERT(center >= 0 && center < 6);
//    }
//  }
//  return false;


//  // ray
//  for (auto p: particles) {
//    switch (sightScope) {
//      case 0:
//        if (p->head.x > head.x && p->head.y == head.y) {
//          return true;
//        }
//        break;
//      case 1:
//        if (p->head.x == head.x && p->head.y > head.y) {
//          return true;
//        }
//        break;
//      case 2:
//        if (p->head.x < head.x && p->head.y > head.y) {
//          return true;
//        }
//        break;
//      case 3:
//        if (p->head.x < head.x && p->head.y == head.y) {
//          return true;
//        }
//        break;
//      case 4:
//        if (p->head.x == head.x && p->head.y < head.y) {
//          return true;
//        }
//        break;
//      case 5:
//        if (p->head.x > head.x && p->head.y < head.y) {
//          return true;
//        }
//        break;
//      default:
//        Q_ASSERT(center >= 0 && center < 6);
//    }
//  }
//  return false;


}

AggregateSystem::AggregateSystem(int numParticles) {
//  _measures.push_back(new MaxDistanceMeasure("MAX 2-Particle Dist", 1, *this));
//  _measures.push_back(new SumDistancesMeasure("SUM 2-Particle Dists", 1, *this));
  _measures.push_back(new ConvexHullMeasure("Convex Hull", 1, *this));
//  _measures.push_back(new MECMeasure("MEC circumference", 1, *this));
  _measures.push_back(new DispersionMeasure("Dispersion", 1, *this));
  _measures.push_back(new ClusterFractionMeasure("Cluster Fraction", 1, *this));

//  currentval = 0;
//  currentval = 1e18;

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




double dist(const QVector<double> a, const QVector<double> b) {
  return sqrt(pow(a[0] - b[0], 2) + pow(a[1] - b[1], 2));
}




//MaxDistanceMeasure::MaxDistanceMeasure(const QString name, const unsigned int freq,
//                                           AggregateSystem& system)
//  : Measure(name, freq),
//    _system(system) {}

//double MaxDistanceMeasure::calculate() const {
//  QVector< QVector<double> > points;

//  for (const auto& p : _system.particles) {
//    auto aggr_p = dynamic_cast<AggregateParticle*>(p);
//    int x_center, y_center;
//    switch (aggr_p->center) {
//      case 0 :
//        x_center = aggr_p->head.x + 1;
//        y_center = aggr_p->head.y;
//        break;
//      case 1 :
//        x_center = aggr_p->head.x;
//        y_center = aggr_p->head.y + 1;
//        break;
//      case 2 :
//        x_center = aggr_p->head.x - 1;
//        y_center = aggr_p->head.y + 1;
//        break;
//      case 3 :
//        x_center = aggr_p->head.x - 1;
//        y_center = aggr_p->head.y;
//        break;
//      case 4 :
//        x_center = aggr_p->head.x;
//        y_center = aggr_p->head.y - 1;
//        break;
//      case 5 :
//        x_center = aggr_p->head.x + 1;
//        y_center = aggr_p->head.y - 1;
//        break;
//    }

//    points.push_back( { ( p->head.x + (p->head.y / 2.0) ),
//                        ( p->head.y * (sqrt(3.0) / 2.0) ) } );
////    points.push_back( { ( x_center + (y_center / 2.0) ) ,
////                        ( y_center * (sqrt(3.0) / 2.0) ) } );
//  }

//  std::sort(points.begin(), points.end());
//  points.erase(std::unique(points.begin(), points.end()), points.end());

//  int n = points.size();

//  double maxDist = 0.0;

//  for (int a = 0; a < n; a++) {
//    for (int b = a + 1; b < n; b++) {
//      if (dist(points[a], points[b]) > maxDist) {
//        maxDist = dist(points[a], points[b]);
//      }
//    }
//  }

//  return maxDist;
//}



//SumDistancesMeasure::SumDistancesMeasure(const QString name, const unsigned int freq,
//                                           AggregateSystem& system)
//  : Measure(name, freq),
//    _system(system) {}

//double SumDistancesMeasure::calculate() const {
//  QVector< QVector<double> > points;

//  for (const auto& p : _system.particles) {
//    auto aggr_p = dynamic_cast<AggregateParticle*>(p);
//    int x_center, y_center;
//    switch (aggr_p->center) {
//      case 0 :
//        x_center = aggr_p->head.x + 1;
//        y_center = aggr_p->head.y;
//        break;
//      case 1 :
//        x_center = aggr_p->head.x;
//        y_center = aggr_p->head.y + 1;
//        break;
//      case 2 :
//        x_center = aggr_p->head.x - 1;
//        y_center = aggr_p->head.y + 1;
//        break;
//      case 3 :
//        x_center = aggr_p->head.x - 1;
//        y_center = aggr_p->head.y;
//        break;
//      case 4 :
//        x_center = aggr_p->head.x;
//        y_center = aggr_p->head.y - 1;
//        break;
//      case 5 :
//        x_center = aggr_p->head.x + 1;
//        y_center = aggr_p->head.y - 1;
//        break;
//    }

//    points.push_back( { ( p->head.x + (p->head.y / 2.0) ),
//                        ( p->head.y * (sqrt(3.0) / 2.0) ) } );
////    points.push_back( { ( x_center + (y_center / 2.0) ) ,
////                        ( y_center * (sqrt(3.0) / 2.0) ) } );
//  }

//  std::sort(points.begin(), points.end());
//  points.erase(std::unique(points.begin(), points.end()), points.end());

//  int n = points.size();

//  double totalDist = 0.0;

//  for (int a = 0; a < n; a++) {
//    for (int b = a + 1; b < n; b++) {
//       totalDist += dist(points[a], points[b]);
//    }
//  }

//  return totalDist;
//}





ConvexHullMeasure::ConvexHullMeasure(const QString name, const unsigned int freq,
                                           AggregateSystem& system)
  : Measure(name, freq),
    _system(system) {}

double ConvexHullMeasure::calculate() const {
  QVector< QVector<double> > hull;

  QVector< QVector<double> > points;

  for (const auto& p : _system.particles) {
    auto aggr_p = dynamic_cast<AggregateParticle*>(p);
    int x_center, y_center;
    switch (aggr_p->center) {
      case 0 :
        x_center = aggr_p->head.x + 1;
        y_center = aggr_p->head.y;
        break;
      case 1 :
        x_center = aggr_p->head.x;
        y_center = aggr_p->head.y + 1;
        break;
      case 2 :
        x_center = aggr_p->head.x - 1;
        y_center = aggr_p->head.y + 1;
        break;
      case 3 :
        x_center = aggr_p->head.x - 1;
        y_center = aggr_p->head.y;
        break;
      case 4 :
        x_center = aggr_p->head.x;
        y_center = aggr_p->head.y - 1;
        break;
      case 5 :
        x_center = aggr_p->head.x + 1;
        y_center = aggr_p->head.y - 1;
        break;
    }

    points.push_back( { ( p->head.x + (p->head.y / 2.0) ),
                        ( p->head.y * (sqrt(3.0) / 2.0) ) } );
//    points.push_back( { ( x_center + (y_center / 2.0) ) ,
//                        ( y_center * (sqrt(3.0) / 2.0) ) } );
  }

  std::sort(points.begin(), points.end());
  points.erase(std::unique(points.begin(), points.end()), points.end());

  int n = points.size();


  int l = 0;
  for (int i = 0; i < n; i++) {
    if (points[i][0] <= points[l][0]) {
      l = i;
    }
  }

  QVector<double> pointOnHull = points[l];
  QVector<double> firstPoint = pointOnHull;
  QVector<double> endpoint;
  QVector<double> beforeInitialPoint;
  double a, b, c, theta, maxTheta;
  QVector< QVector<double> > maxThetaCandidates;

  int i = 0;
  do {
    hull.push_back(pointOnHull);
    if (i != 0) {
      points.erase(std::remove(points.begin(), points.end(), pointOnHull));
    }

    endpoint = points[0];
    maxTheta = 0;
    n = points.size();
    for (int j = 0; j < n; j++) {
      if (i == 0) {
        beforeInitialPoint = { hull[i][0], hull[i][1] - 10.0 };
        a = dist(hull[i], points[j]);
        b = dist(beforeInitialPoint, points[j]);
        c = dist(beforeInitialPoint, hull[i]);
        if (a != 0 && b != 0 && c != 0) {
          if ( (c+a) == b ) {
            theta = M_PI;
          }
          else {
            theta = acos( ((a*a) + (c*c) - (b*b)) / (2 * a * c) );
          }
        }
      } else {
        a = dist(hull[i], points[j]);
        b = dist(hull[i-1], points[j]);
        c = dist(hull[i-1], hull[i]);
        if (a != 0 && b != 0 && c != 0) {
          if ( (c+a) == b ) {
            theta = M_PI;
          }
          else if ( (a+b) == c ) {
            theta = 0.0;
          }
          else {
            if ( ( ((a*a) + (c*c) - (b*b)) / (2 * a * c) ) > 1 &&
                 ( ((a*a) + (c*c) - (b*b)) / (2 * a * c) ) < 1.005 ) {
              theta = 0.0;
            }
            else if ( ( ((a*a) + (c*c) - (b*b)) / (2 * a * c) ) < -1 &&
                      ( ((a*a) + (c*c) - (b*b)) / (2 * a * c) ) > -1.005 ) {
              theta = M_PI;
            }
            else {
              theta = acos( ((a*a) + (c*c) - (b*b)) / (2 * a * c) );
            }
          }
        }

        if (i == 1) {
          if (points[j] == firstPoint) {
            theta = 0.0;
          }
        }
      }

      if (endpoint == pointOnHull) {
        endpoint = points[j];
      }
      else if (theta > maxTheta) {
        maxTheta = theta;
        endpoint = points[j];
        maxThetaCandidates.clear();
        maxThetaCandidates.push_back(points[j]);
      }
      else if (theta == maxTheta) {
        maxThetaCandidates.push_back(points[j]);
        int numCandidates = maxThetaCandidates.size();
        double maximumDist = 0.0;
        for (int z = 0; z < numCandidates; z++) {
          if ( dist(hull[i], maxThetaCandidates[z]) > maximumDist ) {
            endpoint = maxThetaCandidates[z];
          }
        }
      }
    }

    pointOnHull = endpoint;
    i += 1;
  } while (endpoint != hull[0]);

  int hn = hull.size();
  double perimeter = 0.0;
  for (int i = 0; i < hn; i++) {
    if (i == hn-1) {
      perimeter += dist(hull[i], hull[0]);
    } else {
      perimeter += dist(hull[i], hull[i+1]);
    }
  }

//  _system.currentval = perimeter;
  return perimeter;
}




//MECMeasure::MECMeasure(const QString name, const unsigned int freq,
//                                           AggregateSystem& system)
//  : Measure(name, freq),
//    _system(system) {}

//struct Circle {
//    QVector<double> C;
//    double R;
//};

//bool isInside(const Circle& c, const QVector<double> p) {
//  return dist(c.C, p) <= c.R;
//}

//// Find circle given three points that are on it.
//// Uses the fact that the perpendicular bisectors of two chords intersect at the
//// center of the circle.
//Circle circleFromThree(const QVector<double> a, const QVector<double> b,
//                       const QVector<double> c) {
//  double slopeAB, slopeBC, L1slope, L2slope, centerX, centerY = 0.0;

//  // midpoints of the two chords
//  const vector<double> mpAB = { (a[0] + b[0]) / 2.0, (a[1] + b[1]) / 2.0 };
//  const vector<double> mpBC = { (b[0] + c[0]) / 2.0, (b[1] + c[1]) / 2.0 };


//  if (a[0] == b[0] && b[1] == c[1]) {
//    centerX = mpBC[0];
//    centerY = mpAB[1];
//  } else if (b[0] == c[0] && a[1] == b[1]) {
//    centerX = mpAB[0];
//    centerY = mpBC[1];
//  } else if (a[0] == b[0]) {
//    centerY = mpAB[1];
//    slopeBC = (c[1] - b[1]) / (c[0] - b[0]);
//    L2slope = (-1) * (1/slopeBC);
//    centerX = ( (centerY - mpBC[1]) / L2slope ) + mpBC[0];
//  } else if (b[0] == c[0]) {
//    centerY = mpBC[1];
//    slopeAB = (b[1] - a[1]) / (b[0] - a[0]);
//    L1slope = (-1) * (1/slopeAB);
//    centerX = ( (centerY - mpAB[1]) / L1slope) + mpAB[0];
//  } else if (a[1] == b[1]) {
//    centerX = mpAB[0];
//    slopeBC = (c[1] - b[1]) / (c[0] - b[0]);
//    L2slope = (-1) * (1/slopeBC);
//    centerY = ( L2slope * (centerX - mpBC[0]) ) + mpBC[1];
//  } else if (b[1] == c[1]) {
//    centerX = mpBC[0];
//    slopeAB = (b[1] - a[1]) / (b[0] - a[0]);
//    L1slope = (-1) * (1/slopeAB);
//    centerY = ( L1slope * (centerX - mpAB[0]) ) + mpAB[1];
//  } else {
//    slopeAB = (b[1] - a[1]) / (b[0] - a[0]);
//    slopeBC = (c[1] - b[1]) / (c[0] - b[0]);
//    L1slope = (-1) * (1/slopeAB);
//    L2slope = (-1) * (1/slopeBC);
//    centerX = ( mpBC[1] - mpAB[1] + (L1slope * mpAB[0]) - (L2slope * mpBC[0]) ) / (L1slope - L2slope);
//    centerY = ( L1slope * (centerX - mpAB[0]) ) + mpAB[1];
//  }

//  const QVector<double> center{centerX, centerY};

//  return {center, dist(center, b)};
//}

//// Find circle given two points (A, B) that are on it.
//// Returns circle in which AB is the diamter.
//// Therefore, center is midpoint of AB and radius is |AB| / 2.
//Circle circleFromTwo(const QVector<double> a, const QVector<double> b) {
//  const QVector<double> center = { (a[0] + b[0]) / 2.0, (a[1] + b[1]) / 2.0 };
//  return {center, dist(a, b) / 2.0};
//}

//// Checks if all points are inside the circle, therefore making it valid.
//bool isValidCircle(const Circle& c, const QVector< QVector<double> > points) {
//  const int n = points.size();
//  for (int i = 0; i < n; i++) {
//    if (!isInside(c, points[i])) {
//      return false;
//      break;
//    }
//  }
//  return true;
//}


//double MECMeasure::calculate() /*const*/ {
//  QVector< QVector<double> > points;
//  for (const auto& p : _system.particles) {

//    auto aggr_p = dynamic_cast<AggregateParticle*>(p);
//    int x_center, y_center;
//    switch (aggr_p->center) {
//      case 0 :
//        x_center = aggr_p->head.x + 1;
//        y_center = aggr_p->head.y;
//        break;
//      case 1 :
//        x_center = aggr_p->head.x;
//        y_center = aggr_p->head.y + 1;
//        break;
//      case 2 :
//        x_center = aggr_p->head.x - 1;
//        y_center = aggr_p->head.y + 1;
//        break;
//      case 3 :
//        x_center = aggr_p->head.x - 1;
//        y_center = aggr_p->head.y;
//        break;
//      case 4 :
//        x_center = aggr_p->head.x;
//        y_center = aggr_p->head.y - 1;
//        break;
//      case 5 :
//        x_center = aggr_p->head.x + 1;
//        y_center = aggr_p->head.y - 1;
//        break;
//    }

//    points.push_back( { p->head.x + (p->head.y / 2.0),
//                        p->head.y * (sqrt(3.0) / 2.0) } );
////    points.push_back( { x_center + (y_center / 2.0) ,
////                        y_center * (sqrt(3.0) / 2.0) } );
//  }

//  int n = points.size();

//  Circle mec = {{0, 0}, 1e18};

//  // Situation where MEC intersects 2 points (A, B) and AB = diameter.
//  for (int a = 0; a < n; a++) {
//    for (int b = a + 1; b < n; b++) {
//      Circle tmp = circleFromTwo(points[a], points[b]);
//      if (tmp.R < mec.R && isValidCircle(tmp, points)) {
//        mec = tmp;
//      }
//    }
//  }

//  // Situation where MEC intersects 3 or more points (A, B, C).
//  for (int x = 0; x < n; x++) {
//    for (int y = x + 1; y < n; y++) {
//      for (int z = y + 1; z < n; z++) {
//        Circle temp = circleFromThree(points[x], points[y], points[z]);
//        if (temp.R < mec.R && isValidCircle(temp, points)) {
//          mec = temp;
//        }
//      }
//    }
//  }

////  for (int a = 0; a < n; a++) {
////    for (int b = 0; b < n; b++) {
////      Circle tmp = circleFromTwo(points[a], points[b]);
////      if (tmp.R < mec.R && isValidCircle(tmp, points)) {
////        mec = tmp;
////      }
////    }
////  }

////  for (int a = 0; a < n; a++) {
////    for (int b = 0; b < n; b++) {
////      for (int c = 0; c < n; c++) {
////        Circle tmp = circleFromThree(points[a], points[b], points[c]);
////        if (tmp.R < mec.R && isValidCircle(tmp, points)) {
////          mec = tmp;
////        }
////      }
////    }
////  }

//  _system.currentval = ( (mec.R) * 2.0 ) * M_PI;
//  return ( (mec.R) * 2.0 ) * M_PI;
//}




DispersionMeasure::DispersionMeasure(const QString name, const unsigned int freq,
                                           AggregateSystem& system)
  : Measure(name, freq),
    _system(system) {}

double DispersionMeasure::calculate() const {
  QVector< QVector<double> > points;

  for (const auto& p : _system.particles) {
    auto aggr_p = dynamic_cast<AggregateParticle*>(p);
    int x_center, y_center;
    switch (aggr_p->center) {
      case 0 :
        x_center = aggr_p->head.x + 1;
        y_center = aggr_p->head.y;
        break;
      case 1 :
        x_center = aggr_p->head.x;
        y_center = aggr_p->head.y + 1;
        break;
      case 2 :
        x_center = aggr_p->head.x - 1;
        y_center = aggr_p->head.y + 1;
        break;
      case 3 :
        x_center = aggr_p->head.x - 1;
        y_center = aggr_p->head.y;
        break;
      case 4 :
        x_center = aggr_p->head.x;
        y_center = aggr_p->head.y - 1;
        break;
      case 5 :
        x_center = aggr_p->head.x + 1;
        y_center = aggr_p->head.y - 1;
        break;
    }

    points.push_back( { ( p->head.x + (p->head.y / 2.0) ),
                        ( p->head.y * (sqrt(3.0) / 2.0) ) } );
//    points.push_back( { ( x_center + (y_center / 2.0) ) ,
//                        ( y_center * (sqrt(3.0) / 2.0) ) } );
  }

  int n = points.length();

  double x_sum = 0;
  double y_sum = 0;
  for (int i = 0; i < n; i++) {
    x_sum += points[i][0];
    y_sum += points[i][1];
  }
  QVector<double> centroid = {x_sum / n, y_sum / n};

  double dispersion_sum = 0;
  for (int i = 0; i < n; i++) {
    dispersion_sum += dist(centroid, points[i]);
  }

//  _system.currentval = dispersion_sum;
  return dispersion_sum;
}





void AggregateSystem::DFS(AggregateParticle& particle, const AggregateSystem& system,
         std::vector<AggregateParticle>& clusterVec) {
  particle.visited = true;
  clusterVec.push_back(particle);

  for (int j = 0; j < 6; j++) {
    if ( particle.hasNbrAtLabel(j) == true &&
         particle.nbrAtLabel(j).visited == false ) {
      DFS(particle.nbrAtLabel(j), system, clusterVec);
    }
  }
}


ClusterFractionMeasure::ClusterFractionMeasure(const QString name, const unsigned int freq,
                                           AggregateSystem& system)
  : Measure(name, freq),
    _system(system) {}

double ClusterFractionMeasure::calculate() const {
  std::vector< std::vector<AggregateParticle> > allClusterList;

  for (auto& p : _system.particles) {
    auto aggr_p = dynamic_cast<AggregateParticle*>(p);
    aggr_p->visited = false;
  }

  for (auto& p : _system.particles) {
    auto aggr_p = dynamic_cast<AggregateParticle*>(p);
    if (aggr_p->visited == false) {
      std::vector<AggregateParticle> currentCluster = {};
      _system.DFS(*aggr_p, _system, currentCluster);
      allClusterList.push_back(currentCluster);
    }
  }


  int numInMaxCluster = allClusterList[0].size();
  int cn = allClusterList.size();
  for (int i = 0; i < cn; i++) {
    if (int( allClusterList[i].size() ) > numInMaxCluster) {
      numInMaxCluster = allClusterList[i].size();
    }
  }

  double double_numInMaxCluster = numInMaxCluster;
  double double_systemSize = _system.size();
//  _system.currentval = (double_numInMaxCluster / double_systemSize);
  return (double_numInMaxCluster / double_systemSize);
}









bool AggregateSystem::hasTerminated() const {
//  double num_particles = size();

////  // convex hull per.
////  double ideal_cluster = 1.959050785 * pow(num_particles, 0.603108546);

//  // dispersion
//  int num_rings, ideal_cluster;
//  if (num_particles >= 169) {
//   num_rings = 7;
//   ideal_cluster = (0*1) + (1*6) + (2*12) + (3*18) + (4*24) + (5*30) + (6*36) + (7*42) + ( (num_particles-169) * (num_rings+1) );
//  } else if (num_particles >= 127) {
//   num_rings = 6;
//   ideal_cluster = (0*1) + (1*6) + (2*12) + (3*18) + (4*24) + (5*30) + (6*36) + ( (num_particles-127) * (num_rings+1) );
//  } else if (num_particles >= 91) {
//   num_rings = 5;
//   ideal_cluster = (0*1) + (1*6) + (2*12) + (3*18) + (4*24) + (5*30) + ( (num_particles-91) * (num_rings+1) );
//  } else if (num_particles >= 61) {
//   num_rings = 4;
//   ideal_cluster = (0*1) + (1*6) + (2*12) + (3*18) + (4*24) + ( (num_particles-61) * (num_rings+1) );
//  } else if (num_particles >= 37) {
//   num_rings = 3;
//   ideal_cluster = (0*1) + (1*6) + (2*12) + (3*18) + ( (num_particles-31) * (num_rings+1) );
//  } else if (num_particles >= 19) {
//   num_rings = 2;
//   ideal_cluster = (0*1) + (1*6) + (2*12) + ( (num_particles-19) * (num_rings+1) );
//  } else if (num_particles >= 7) {
//   num_rings = 1;
//   ideal_cluster = (0*1) + (1*6) + ( (num_particles-7) * (num_rings+1) );
//  } else {
//   num_rings = 0;
//   ideal_cluster = (num_particles-1) * 1;
//  }

//  if ( currentval <= (1.15 * ideal_cluster) ) {
//    return true;
//  } else {
//    return false;
//  }



//  // mec circumf.
//  double ideal_cluster = 2.033785231 * pow(num_particles, 0.6059016911);
//  if ( currentval <= (1.25 * ideal_cluster) ) {
//    return true;
//  } else {
//    return false;
//  }


//  // clust frac.
//  if ( currentval >= 1) {
//    return true;
//  } else {
//    return false;
//  }


  return false;

}
