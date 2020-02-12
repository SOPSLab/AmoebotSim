/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "core/localparticle.h"

const std::vector<int> LocalParticle::sixLabels = {
  {0, 1, 2, 3, 4, 5}
};

const std::array<const std::vector<int>, 6> LocalParticle::labels = {
  {{3, 4, 5, 6, 7},
   {4, 5, 6, 7, 8},
   {7, 8, 9, 0, 1},
   {8, 9, 0, 1, 2},
   {9, 0, 1, 2, 3},
   {2, 3, 4, 5, 6}}
};

const std::array<int, 6> LocalParticle::contractLabels = {
  {0, 1, 4, 5, 6, 9}
};

const std::array<std::array<int, 10>, 6> LocalParticle::labelDir = {
  {{{0, 1, 2, 1, 2, 3, 4, 5, 4, 5}},
   {{0, 1, 2, 3, 2, 3, 4, 5, 0, 5}},
   {{0, 1, 0, 1, 2, 3, 4, 3, 4, 5}},
   {{0, 1, 2, 1, 2, 3, 4, 5, 4, 5}},
   {{0, 1, 2, 3, 2, 3, 4, 5, 0, 5}},
   {{0, 1, 0, 1, 2, 3, 4, 3, 4, 5}}}
};

LocalParticle::LocalParticle(const Node& head, int globalTailDir,
                             const int orientation)
  : Particle(head, globalTailDir),
  orientation(orientation) {
  Q_ASSERT(0 <= orientation && orientation < 6);
}

int LocalParticle::tailDir() const {
  Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);

  return isContracted() ? -1 : globalToLocalDir(globalTailDir);
}

int LocalParticle::labelToDir(int label) const {
  Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);

  if (isContracted()) {
    Q_ASSERT(0 <= label && label < 6);
    return label;
  } else {
    Q_ASSERT(0 <= label && label < 10);
    return labelDir[tailDir()][label];
  }
}

int LocalParticle::labelToDirAfterExpansion(int label, int expansionDir) const {
  Q_ASSERT(isContracted());
  Q_ASSERT(0 <= label && label < 10);
  Q_ASSERT(0 <= expansionDir && expansionDir < 6);

  return labelDir[(expansionDir + 3) % 6][label];
}

const std::vector<int> LocalParticle::uniqueLabels() const {
  std::vector<int> labels;
  if (isContracted()) {
    labels = sixLabels;
  } else { // isExpanded().
    for (int label = 0; label < 10; ++label) {
      if (nbrNodeReachedViaLabel(label) !=
          nbrNodeReachedViaLabel((label + 9) % 10)) {
        labels.push_back(label);
      }
    }
  }

  return labels;
}

const std::vector<int>& LocalParticle::headLabels() const {
  Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);

  return isContracted() ? sixLabels : labels[tailDir()];
}

const std::vector<int>& LocalParticle::tailLabels() const {
  Q_ASSERT(isExpanded());

  return labels[(tailDir() + 3) % 6];
}

bool LocalParticle::isHeadLabel(int label) const {
  Q_ASSERT(0 <= label && label < 10);

  for (const int headLabel : headLabels()) {
    if (label == headLabel) {
      return true;
    }
  }

  return false;
}

bool LocalParticle::isTailLabel(int label) const {
  Q_ASSERT(isExpanded());
  Q_ASSERT(0 <= label && label < 10);

  for (const int tailLabel : tailLabels()) {
    if (label == tailLabel) {
      return true;
    }
  }

  return false;
}

int LocalParticle::dirToHeadLabel(int dir) const {
  Q_ASSERT(0 <= dir && dir < 6);

  for (const int headLabel : headLabels()) {
    if (dir == labelToDir(headLabel)) {
      return headLabel;
    }
  }

  Q_ASSERT(false);
  return 0;  // Avoid compiler warning.
}

int LocalParticle::dirToTailLabel(int dir) const {
  Q_ASSERT(isExpanded());
  Q_ASSERT(0 <= dir && dir < 6);

  for (const int tailLabel : tailLabels()) {
    if (dir == labelToDir(tailLabel)) {
      return tailLabel;
    }
  }

  Q_ASSERT(false);
  return 0;  // Avoid compiler warning.
}

int LocalParticle::headContractionLabel() const {
  Q_ASSERT(isExpanded());

  return contractLabels[tailDir()];
}

int LocalParticle::tailContractionLabel() const {
  Q_ASSERT(isExpanded());

  return contractLabels[(tailDir() + 3) % 6];
}

const std::vector<int>& LocalParticle::headLabelsAfterExpansion(
    int expansionDir) const {
  Q_ASSERT(isContracted());
  Q_ASSERT(0 <= expansionDir && expansionDir < 6);

  const int tempTailDir = (expansionDir + 3) % 6;
  return labels[tempTailDir];
}

const std::vector<int>& LocalParticle::tailLabelsAfterExpansion(
    int expansionDir) const {
  Q_ASSERT(isContracted());
  Q_ASSERT(0 <= expansionDir && expansionDir < 6);

  const int tempTailDir = (expansionDir + 3) % 6;
  return labels[(tempTailDir + 3) % 6];
}

bool LocalParticle::isHeadLabelAfterExpansion(int label, int expansionDir)
    const {
  Q_ASSERT(isContracted());
  Q_ASSERT(0 <= expansionDir && expansionDir < 6);

  for (const int headLabel : headLabelsAfterExpansion(expansionDir)) {
    if (label == headLabel) {
      return true;
    }
  }

  return false;
}

bool LocalParticle::isTailLabelAfterExpansion(int label, int expansionDir)
    const {
  Q_ASSERT(isContracted());
  Q_ASSERT(0 <= expansionDir && expansionDir < 6);

  for (const int tailLabel : tailLabelsAfterExpansion(expansionDir)) {
    if (label == tailLabel) {
      return true;
    }
  }

  return false;
}

int LocalParticle::dirToHeadLabelAfterExpansion(int dir, int expansionDir)
    const {
  Q_ASSERT(isContracted());
  Q_ASSERT(0 <= dir && dir < 6);
  Q_ASSERT(0 <= expansionDir && expansionDir < 6);

  for (const int headLabel : headLabelsAfterExpansion(expansionDir)) {
    if (dir == labelToDirAfterExpansion(headLabel, expansionDir)) {
      return headLabel;
    }
  }

  Q_ASSERT(false);
  return 0;  // Avoid compiler warning.
}

int LocalParticle::dirToTailLabelAfterExpansion(int dir, int expansionDir)
    const {
  Q_ASSERT(isContracted());
  Q_ASSERT(0 <= dir && dir < 6);
  Q_ASSERT(0 <= expansionDir && expansionDir < 6);

  for (const int tailLabel : tailLabelsAfterExpansion(expansionDir)) {
    if (dir == labelToDirAfterExpansion(tailLabel, expansionDir)) {
      return tailLabel;
    }
  }

  Q_ASSERT(false);
  return 0;  // Avoid compiler warning.
}

int LocalParticle::headContractionLabelAfterExpansion(int expansionDir) const {
  Q_ASSERT(isContracted());
  Q_ASSERT(0 <= expansionDir && expansionDir < 6);

  return contractLabels[(expansionDir + 3) % 6];
}

int LocalParticle::tailContractionLabelAfterExpansion(int expansionDir) const {
  Q_ASSERT(isContracted());
  Q_ASSERT(0 <= expansionDir && expansionDir < 6);

  return contractLabels[expansionDir];
}

int LocalParticle::labelToGlobalDir(int label) const {
  Q_ASSERT(0 <= label && label < 10);

  return localToGlobalDir(labelToDir(label));
}

int LocalParticle::labelOfNbrNodeInGlobalDir(const Node& node, int globalDir)
    const {
  Q_ASSERT(0 <= globalDir && globalDir < 6);

  const int labelLimit = (isContracted()) ? 6 : 10;
  for (int label = 0; label < labelLimit; label++) {
    if (labelToGlobalDir(label) == globalDir &&
        nbrNodeReachedViaLabel(label) == node) {
      return label;
    }
  }

  Q_ASSERT(false);
  return 0;  // Avoid compiler warning.
}

Node LocalParticle::occupiedNodeIncidentToLabel(int label) const {
  Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);

  if (isContracted()) {
    Q_ASSERT(0 <= label && label < 6);
    return head;
  } else {
    Q_ASSERT(0 <= label && label < 10);
    return isHeadLabel(label) ? head : tail();
  }
}

Node LocalParticle::nbrNodeReachedViaLabel(int label) const {
  Q_ASSERT(-1 <= globalTailDir && globalTailDir < 6);
  if (isContracted()) {
    Q_ASSERT(0 <= label && label < 6);
    const int dir = (orientation + label) % 6;
    return head.nodeInDir(dir);
  } else {
    Q_ASSERT(0 <= label && label < 10);
    Node incidentNode = occupiedNodeIncidentToLabel(label);
    return incidentNode.nodeInDir(labelToGlobalDir(label));
  }
}

int LocalParticle::localToGlobalDir(int localDir) const {
  Q_ASSERT(0 <= localDir && localDir < 6);

  return (orientation + localDir) % 6;
}

int LocalParticle::globalToLocalDir(int globalDir) const {
  Q_ASSERT(0 <= globalDir && globalDir < 6);

  return (globalDir - orientation + 6) % 6;
}

int LocalParticle::nbrDirToDir(const LocalParticle& nbr, int nbrDir) const {
  Q_ASSERT(0 <= nbrDir && nbrDir < 6);

  return globalToLocalDir(nbr.localToGlobalDir(nbrDir));
}

int LocalParticle::dirToNbrDir(const LocalParticle& nbr, int myDir) const {
  Q_ASSERT(0 <= myDir && myDir < 6);

  return nbr.globalToLocalDir(localToGlobalDir(myDir));
}

bool LocalParticle::pointsAtMe(const LocalParticle& nbr, int nbrLabel) const {
  Q_ASSERT(0 <= nbrLabel && nbrLabel < 10);

  if (isContracted()) {
    return pointsAtMyHead(nbr, nbrLabel);
  } else {
    return pointsAtMyHead(nbr, nbrLabel) || pointsAtMyTail(nbr, nbrLabel);
  }
}

bool LocalParticle::pointsAtMyHead(const LocalParticle& nbr, int nbrLabel)
    const {
  Q_ASSERT(0 <= nbrLabel && nbrLabel < 10);

  return nbr.nbrNodeReachedViaLabel(nbrLabel) == head;
}

bool LocalParticle::pointsAtMyTail(const LocalParticle& nbr, int nbrLabel)
    const {
  Q_ASSERT(isExpanded());
  Q_ASSERT(0 <= nbrLabel && nbrLabel < 10);

  return nbr.nbrNodeReachedViaLabel(nbrLabel) == tail();
}
