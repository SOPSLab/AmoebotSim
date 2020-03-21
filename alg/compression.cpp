/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "alg/compression.h"

#include <algorithm>  // For distance() and find().
#include <set>
#include <vector>

#include <QtGlobal>

CompressionParticle::CompressionParticle(const Node head,
                                         const int globalTailDir,
                                         const int orientation,
                                         AmoebotSystem& system,
                                         const double lambda)
  : AmoebotParticle(head, globalTailDir, orientation, system),
    lambda(lambda),
    q(0),
    numNbrsBefore(0),
    flag(false) {}

void CompressionParticle::activate() {
  if (isContracted()) {
    int expandDir = randDir();  // Select a random neighboring location.
    q = randDouble(0, 1);        // Select a random q in (0,1).

    if (canExpand(expandDir) && !hasExpNbr()) {
      // Count neighbors in original position and expand.
      numNbrsBefore = nbrCount(uniqueLabels());
      expand(expandDir);
      flag = !hasExpNbr();
    }
  } else {  // isExpanded().
    if (!flag || numNbrsBefore == 5) {
      contractHead();
    } else {
      // Count neighbors in new position and compute the set S.
      int numNbrsAfter = nbrCount(headLabels());
      std::vector<int> S;
      for (const int label : {headLabels()[4], tailLabels()[4]}) {
        if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
          S.push_back(label);
        }
      }

      // If the conditions are satisfied, contract to the new position;
      // otherwise, contract back to the original one.
      if ((q < pow(lambda, numNbrsAfter - numNbrsBefore))
          && (checkProp1(S) || checkProp2(S))) {
        contractTail();
      } else {
        contractHead();
      }
    }
  }
}

QString CompressionParticle::inspectionText() const {
  QString text;
  text += "Global Info:\n";
  text += "  head: (" + QString::number(head.x) + ", "
                      + QString::number(head.y) + ")\n";
  text += "  orientation: " + QString::number(orientation) + "\n";
  text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
  text += "Properties:\n";
  text += "  lambda = " + QString::number(lambda) + ",\n";
  text += "  q in (0,1) = " + QString::number(q) + ",\n";
  text += "  flag = " + QString::number(flag) + ".\n";

  if(isContracted()) {
    text += "Contracted properties:\n";
    text += "  #neighbors before = " + QString::number(numNbrsBefore) + ",\n";
  } else {  // isExpanded().
    text += "Expanded properties:\n";
    text += "  #neighbors before = " + QString::number(numNbrsBefore) + ",\n";
    text += "  #neighbors after = " + QString::number(nbrCount(headLabels()))
            + ".\n";
  }

  return text;
}

CompressionParticle& CompressionParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<CompressionParticle>(label);
}

bool CompressionParticle::hasExpNbr() const {
  for (const int label: uniqueLabels()) {
    if (hasNbrAtLabel(label) && nbrAtLabel(label).isExpanded()) {
      return true;
    }
  }

  return false;
}

bool CompressionParticle::hasExpHeadAtLabel(const int label) const {
  return hasNbrAtLabel(label) && nbrAtLabel(label).isExpanded()
         && nbrAtLabel(label).pointsAtMyHead(*this, label);
}

int CompressionParticle::nbrCount(std::vector<int> labels) const {
  int numNbrs = 0;
  for (const int label : labels) {
    if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
      ++numNbrs;
    }
  }

  return numNbrs;
}

bool CompressionParticle::checkProp1(std::vector<int> S) const {
  Q_ASSERT(isExpanded());
  Q_ASSERT(S.size() <= 2);
  Q_ASSERT(flag);  // Not required, but equivalent/cleaner for implementation.

  if (S.size() == 0) {
    return false;  // S has to be nonempty for Property 1.
  } else {
    const std::vector<int> labels = uniqueLabels();
    std::set<int> adjNbrs;

    // Starting from the particles in S, sweep out and mark connected neighbors.
    for (int s : S) {
      adjNbrs.insert(s);
      int i = distance(labels.begin(), find(labels.begin(), labels.end(), s));

      // First sweep counter-clockwise, stopping when an unoccupied position or
      // expanded head is encountered.
      for (uint offset = 1; offset < labels.size(); ++offset) {
        int label = labels[(i + offset) % labels.size()];
        if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
          adjNbrs.insert(label);
        } else {
          break;
        }
      }

      // Then sweep clockwise.
      for (uint offset = 1; offset < labels.size(); ++offset) {
        int label = labels[(i - offset + labels.size()) % labels.size()];
        if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
          adjNbrs.insert(label);
        } else {
          break;
        }
      }
    }

    // If all neighbors are connected to a particle in S by a path through the
    // neighborhood, then the number of labels in adjNbrs should equal the total
    // number of neighbors.
    return adjNbrs.size() == (uint)nbrCount(labels);
  }
}

bool CompressionParticle::checkProp2(std::vector<int> S) const {
  Q_ASSERT(isExpanded());
  Q_ASSERT(S.size() <= 2);
  Q_ASSERT(flag);  // Not required, but equivalent/cleaner for implementation.

  if (S.size() != 0) {
    return false;  // S has to be empty for Property 2.
  } else {
    const int numHeadNbrs = nbrCount(headLabels());
    const int numTailNbrs = nbrCount(tailLabels());

    // Check if the head's neighbors are connected.
    int numAdjHeadNbrs = 0;
    bool seenNbr = false;
    for (const int label : headLabels()) {
      if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
        seenNbr = true;
        ++numAdjHeadNbrs;
      } else if (seenNbr) {
        break;
      }
    }

    // Check if the tail's neighbors are connected.
    int numAdjTailNbrs = 0;
    seenNbr = false;
    for (const int label : tailLabels()) {
      if (hasNbrAtLabel(label) && !hasExpHeadAtLabel(label)) {
        seenNbr = true;
        ++numAdjTailNbrs;
      } else if (seenNbr) {
        break;
      }
    }

    // Property 2 is satisfied if both the head and tail have at least one
    // neighbor and all head (tail) neighbors are connected.
    return (numHeadNbrs > 0) && (numTailNbrs > 0) &&
           (numHeadNbrs == numAdjHeadNbrs) && (numTailNbrs == numAdjTailNbrs);
  }
}


CompressionSystem::CompressionSystem(int numParticles, double lambda) {
  Q_ASSERT(lambda > 1);

  // Initialize particle system.
  if (lambda <= 2.17) {  // In the proven range of expansion, make a hexagon.
    int x, y;
    for (int i = 1; i <= numParticles; ++i) {
      int layer = 1;
      int position = i - 1;
      while (position - (6 * layer) >= 0) {
        position -= 6 * layer;
        ++layer;
      }

      switch(position / layer) {
        case 0: {
          x = layer;
          y = (position % layer) - layer;
          if (position % layer == 0) {x -= 1; y += 1;}  // Corner case.
          break;
        }
        case 1: {
          x = layer - (position % layer);
          y = position % layer;
          break;
        }
        case 2: {
          x = -1 * (position % layer);
          y = layer;
          break;
        }
        case 3: {
          x = -1 * layer;
          y = layer - (position % layer);
          break;
        }
        case 4: {
          x = (position % layer) - layer;
          y = -1 * (position % layer);
          break;
        }
        case 5: {
          x = (position % layer);
          y = -1 * layer;
          break;
        }
      }

      insert(new CompressionParticle(Node(x, y), -1, randDir(), *this, lambda));
    }
  } else {  // In the unknown range or compression range, make a straight line.
    for (int i = 0; i < numParticles; ++i) {
      insert(new CompressionParticle(Node(i, 0), -1, randDir(), *this, lambda));
    }
  }

  // Set up metrics.
  _measures.push_back(new PerimeterMeasure("Perimeter", 1, *this));
}

bool CompressionSystem::hasTerminated() const {
  #ifdef QT_DEBUG
    if (!isConnected(particles)) {
        return true;
    }
  #endif

  return false;
}

PerimeterMeasure::PerimeterMeasure(const QString name, const unsigned int freq,
                                   CompressionSystem& system)
    : Measure(name, freq),
      _system(system) {}

double PerimeterMeasure::calculate() const {
  int numEdges = 0;
  for (const auto& p : _system.particles) {
    auto comp_p = dynamic_cast<CompressionParticle*>(p);
    auto tailLabels = comp_p->isContracted() ? comp_p->uniqueLabels()
                                             : comp_p->tailLabels();
    for (const int label : tailLabels) {
      if (comp_p->hasNbrAtLabel(label) && !comp_p->hasExpHeadAtLabel(label)) {
        ++numEdges;
      }
    }
  }

  return (3 * _system.size()) - (numEdges / 2) - 3;
}
