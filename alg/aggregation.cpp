#include <QDebug>

#include <math.h>
#include <unordered_set>

#include "aggregation.h"

using namespace std;

AggregateParticle::AggregateParticle(const Node head, const int globalTailDir,
                                     const int orientation,
                                     AmoebotSystem& system,
                                     int center, QString mode, double noiseVal,
                                     std::vector<AggregateParticle*> particles)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    center(center),
    mode(mode),
    noiseVal(noiseVal),
    particles(particles) {}

void AggregateParticle::activate() {
  bool particleInSight = checkIfParticleInSight();

  if (mode == "d") {    // Deadlock perturbation noise.
    int perturbCounterMax = noiseVal;

    if (particleInSight) {
      // Particle in sight, rotate clockwise in place.
      center = (center + 5) % 6;
      perturb = 0;
    } else {
      // No particle in sight, move clockwise around center of rotation (taking
      // into account perturbation rules).
      int moveDir = (center + 1) % 6;
      if (!hasNbrAtLabel(moveDir)) {
        expand(moveDir);
        contractTail();
        center = (center + 5) % 6;
      } else {
        perturb++;
        if (perturb >= perturbCounterMax) {
          center = (center + 5) % 6;
          perturb = 0;
        }
      }
    }
  } else {    // Mode = "e", error probability noise.
    if (randBool(noiseVal)) {
      particleInSight = !particleInSight;
    }

    if (particleInSight) {
      // "Particle in sight", rotate clockwise in place.
      center = (center + 5) % 6;
    } else {
      // "No particle in sight", move clockwise around center of rotation.
      int moveDir = (center + 1) % 6;
      if (!hasNbrAtLabel(moveDir)) {
        expand(moveDir);
        contractTail();
        center = (center + 5) % 6;
      }
    }
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
  text += "head: (" + QString::number(head.x) + ", " + QString::number(head.y)
          + ")\n";
  text += "orientation: " + QString::number(orientation) + "\n";
  text += "globalTailDir: " + QString::number(globalTailDir) + "\n";
  text += "\n";
  text += "center: " + QString::number(center) + "\n";
  text += "sight: " + QString::number((center + 5) % 6) + "\n";
  text += "particle in sight: "
          + QString::number(this->checkIfParticleInSight()) + "\n";
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
}

AggregateSystem::AggregateSystem(int numParticles, QString mode,
                                 double noiseVal) {
  _measures.push_back(new SEDMeasure("SED circumference", 1, *this));
  _measures.push_back(new ConvexHullMeasure("Convex Hull Perim", 1, *this));
  _measures.push_back(new DispersionMeasure("Dispersion", 1, *this));
  _measures.push_back(new ClusterFractionMeasure("Cluster Fraction", 1, *this));

  Q_ASSERT(mode == "d" or mode == "e");
  Q_ASSERT(noiseVal >= 0);
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
          new AggregateParticle(Node(x, y), -1, 0, *this, randDir(), mode,
                                noiseVal, particles);
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

struct Circle {
    QVector<double> C;
    double R;
};

bool isInside(const Circle& c, const QVector<double> p) {
  return dist(c.C, p) <= c.R;
}

Circle circleFromThree(const QVector<double> a, const QVector<double> b,
                       const QVector<double> c) {
  double slopeAB, slopeBC, L1slope, L2slope, centerX, centerY = 0.0;

  const vector<double> mpAB = { (a[0] + b[0]) / 2.0, (a[1] + b[1]) / 2.0 };
  const vector<double> mpBC = { (b[0] + c[0]) / 2.0, (b[1] + c[1]) / 2.0 };

  if (a[0] == b[0] && b[1] == c[1]) {
    centerX = mpBC[0];
    centerY = mpAB[1];
  } else if (b[0] == c[0] && a[1] == b[1]) {
    centerX = mpAB[0];
    centerY = mpBC[1];
  } else if (a[0] == b[0]) {
    centerY = mpAB[1];
    slopeBC = (c[1] - b[1]) / (c[0] - b[0]);
    L2slope = (-1) * (1/slopeBC);
    centerX = ((centerY - mpBC[1]) / L2slope) + mpBC[0];
  } else if (b[0] == c[0]) {
    centerY = mpBC[1];
    slopeAB = (b[1] - a[1]) / (b[0] - a[0]);
    L1slope = (-1) * (1/slopeAB);
    centerX = ((centerY - mpAB[1]) / L1slope) + mpAB[0];
  } else if (a[1] == b[1]) {
    centerX = mpAB[0];
    slopeBC = (c[1] - b[1]) / (c[0] - b[0]);
    L2slope = (-1) * (1/slopeBC);
    centerY = (L2slope * (centerX - mpBC[0])) + mpBC[1];
  } else if (b[1] == c[1]) {
    centerX = mpBC[0];
    slopeAB = (b[1] - a[1]) / (b[0] - a[0]);
    L1slope = (-1) * (1/slopeAB);
    centerY = (L1slope * (centerX - mpAB[0])) + mpAB[1];
  } else {
    slopeAB = (b[1] - a[1]) / (b[0] - a[0]);
    slopeBC = (c[1] - b[1]) / (c[0] - b[0]);
    L1slope = (-1) * (1/slopeAB);
    L2slope = (-1) * (1/slopeBC);
    centerX = (mpBC[1] - mpAB[1] + (L1slope * mpAB[0]) - (L2slope * mpBC[0]))
              / (L1slope - L2slope);
    centerY = (L1slope * (centerX - mpAB[0])) + mpAB[1];
  }

  const QVector<double> center{centerX, centerY};

  return {center, dist(center, b)};
}

Circle circleFromTwo(const QVector<double> a, const QVector<double> b) {
  const QVector<double> center = {(a[0] + b[0]) / 2.0, (a[1] + b[1]) / 2.0};
  return {center, dist(a, b) / 2.0};
}

bool isValidCircle(const Circle& c, const QVector<QVector<double>> points) {
  const int n = points.size();
  for (int i = 0; i < n; i++) {
    if (!isInside(c, points[i])) {
      return false;
      break;
    }
  }
  return true;
}

Circle minCircleTrivial(QVector<QVector<double>>& points) {
  assert(points.size() <= 3);
  if (points.empty()) {
    return {{0, 0}, 0};
  } else if (points.size() == 1) {
    return {points[0], 0};
  } else if (points.size() == 2) {
    return circleFromTwo(points[0], points[1]);
  }

  for (int i = 0; i < 3; i++) {
    for (int j = i + 1; j < 3; j++) {
      Circle c = circleFromTwo(points[i], points[j]);
      if (isValidCircle(c, points)) {
        return c;
      }
    }
  }

  return circleFromThree(points[0], points[1], points[2]);
}

// Returns the smallest enclosing disc (SED) using Welzl's algorithm.
// P = set of input points, R = set of points on circle boundary, n = number of
// points in P not yet processed.
Circle welzlHelper(QVector<QVector<double>>& P, QVector<QVector<double>> R,
                   int n) {
  if (n == 0 || R.size() == 3) {
    return minCircleTrivial(R);
  }

  int idx = rand() % n;
  QVector<double> p = P[idx];

  swap(P[idx], P[n - 1]);

  Circle d = welzlHelper(P, R, n - 1);

  if (isInside(d, p)) {
    return d;
  }
  R.push_back(p);

  return welzlHelper(P, R, n - 1);
}

Circle welzl(const QVector<QVector<double>>& P) {
  QVector<QVector<double>> P_copy = P;
  random_shuffle(P_copy.begin(), P_copy.end());
  return welzlHelper(P_copy, {}, P_copy.size());
}

SEDMeasure::SEDMeasure(const QString name, const unsigned int freq,
                       AggregateSystem& system)
  : Measure(name, freq),
    _system(system) {}

double SEDMeasure::calculate() const {
  QVector<QVector<double>> points = {};
  for (const auto& p : _system.particles) {
    points.push_back({(p->head.x + (p->head.y / 2.0)),
                      (p->head.y * (sqrt(3.0) / 2.0))});
  }

  Circle sed = welzl(points);

  return sed.R * 2.0 * M_PI;
}

ConvexHullMeasure::ConvexHullMeasure(const QString name, const unsigned int freq,
                                     AggregateSystem& system)
  : Measure(name, freq),
    _system(system) {}

// Returns the perimeter of the convex hull of the system using the gift
// wrapping algorithm.
double ConvexHullMeasure::calculate() const {
  QVector<QVector<double>> hull;

  QVector<QVector<double>> points;

  for (const auto& p : _system.particles) {
    points.push_back({(p->head.x + (p->head.y / 2.0)),
                      (p->head.y * (sqrt(3.0) / 2.0))});
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
  QVector<QVector<double>> maxThetaCandidates;

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
        beforeInitialPoint = {hull[i][0], hull[i][1] - 10.0};
        a = dist(hull[i], points[j]);
        b = dist(beforeInitialPoint, points[j]);
        c = dist(beforeInitialPoint, hull[i]);
        if (a != 0 && b != 0 && c != 0) {
          if ((c + a) == b) {
            theta = M_PI;
          } else {
            theta = acos(((a*a) + (c*c) - (b*b)) / (2 * a * c));
          }
        }
      } else {
        a = dist(hull[i], points[j]);
        b = dist(hull[i-1], points[j]);
        c = dist(hull[i-1], hull[i]);
        if (a != 0 && b != 0 && c != 0) {
          if ((c + a) == b) {
            theta = M_PI;
          } else if ((a + b) == c) {
            theta = 0.0;
          } else {
            if ((((a*a) + (c*c) - (b*b)) / (2 * a * c)) > 1 &&
                (((a*a) + (c*c) - (b*b)) / (2 * a * c)) < 1.005) {
              theta = 0.0;
            } else if ((((a*a) + (c*c) - (b*b)) / (2 * a * c)) < -1 &&
                       (((a*a) + (c*c) - (b*b)) / (2 * a * c)) > -1.005) {
              theta = M_PI;
            } else {
              theta = acos(((a*a) + (c*c) - (b*b)) / (2 * a * c));
            }
          }
        }

        if (i == 1 && points[j] == firstPoint) {
          theta = 0.0;
        }
      }

      if (endpoint == pointOnHull) {
        endpoint = points[j];
      } else if (theta > maxTheta) {
        maxTheta = theta;
        endpoint = points[j];
        maxThetaCandidates.clear();
        maxThetaCandidates.push_back(points[j]);
      } else if (theta == maxTheta) {
        maxThetaCandidates.push_back(points[j]);
        int numCandidates = maxThetaCandidates.size();
        double maximumDist = 0.0;
        for (int z = 0; z < numCandidates; z++) {
          if (dist(hull[i], maxThetaCandidates[z]) > maximumDist) {
            endpoint = maxThetaCandidates[z];
          }
        }
      }
    }

    pointOnHull = endpoint;
    i++;
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

  return perimeter;
}

DispersionMeasure::DispersionMeasure(const QString name, const unsigned int freq,
                                     AggregateSystem& system)
  : Measure(name, freq),
    _system(system) {}

double DispersionMeasure::calculate() const {
  QVector< QVector<double> > points;

  for (const auto& p : _system.particles) {
    points.push_back({(p->head.x + (p->head.y / 2.0)),
                      (p->head.y * (sqrt(3.0) / 2.0))});
  }

  int n = points.length();
  double xSum = 0;
  double ySum = 0;
  for (int i = 0; i < n; i++) {
    xSum += points[i][0];
    ySum += points[i][1];
  }
  QVector<double> centroid = {xSum / n, ySum / n};

  double dispersionSum = 0;
  for (int i = 0; i < n; i++) {
    dispersionSum += dist(centroid, points[i]);
  }

  return dispersionSum;
}

void AggregateSystem::DFS(AggregateParticle& particle,
                          const AggregateSystem& system,
                          std::vector<AggregateParticle>& clusterVec) {
  particle.visited = true;
  clusterVec.push_back(particle);

  for (int j = 0; j < 6; j++) {
    if (particle.hasNbrAtLabel(j) == true &&
        particle.nbrAtLabel(j).visited == false) {
      DFS(particle.nbrAtLabel(j), system, clusterVec);
    }
  }
}

ClusterFractionMeasure::ClusterFractionMeasure(const QString name,
                                               const unsigned int freq,
                                               AggregateSystem& system)
  : Measure(name, freq),
    _system(system) {}

double ClusterFractionMeasure::calculate() const {
  std::vector<std::vector<AggregateParticle>> allClusterList;

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
    if (int(allClusterList[i].size()) > numInMaxCluster) {
      numInMaxCluster = allClusterList[i].size();
    }
  }

  double doubleNumInMaxCluster = numInMaxCluster;
  double doubleSystemSize = _system.size();
  return (doubleNumInMaxCluster / doubleSystemSize);
}

bool AggregateSystem::hasTerminated() const {
  return false;
}
