#include <QtGlobal>

#include "sim/particle.h"

Particle::Particle(Algorithm* _algorithm, const int _orientation, const Node _head, const int _tailDir)
    : orientation(_orientation),
      head(_head),
      tailDir(_tailDir),
      algorithm(_algorithm),
      newAlgorithm(nullptr)
{
    Q_ASSERT(_algorithm != nullptr);
}

Particle::Particle(const Particle& other)
    : orientation(other.orientation),
      head(other.head),
      tailDir(other.tailDir),
      algorithm(other.algorithm->clone()),
      newAlgorithm(nullptr)
{
}

Particle::~Particle()
{
    delete algorithm;
    delete newAlgorithm;
}

Particle& Particle::operator=(const Particle& other)
{
    delete algorithm;
    orientation = other.orientation;
    head = other.head;
    tailDir = other.tailDir;
    algorithm = other.algorithm->clone();
    return *this;
}

Movement Particle::executeAlgorithm(std::array<const Flag*, 10>& inFlag)
{
    Q_ASSERT(newAlgorithm == nullptr);
    newAlgorithm = algorithm->clone();
    return newAlgorithm->delegateExecute(inFlag);
}

void Particle::apply()
{
    Q_ASSERT(newAlgorithm != nullptr);
    delete algorithm;
    algorithm = newAlgorithm;
    newAlgorithm = nullptr;
}

void Particle::discard()
{
    Q_ASSERT(newAlgorithm != nullptr);
    delete newAlgorithm;
    newAlgorithm = nullptr;
}

const Flag* Particle::getFlagForNode(Node node)
{
    return algorithm->outFlags[labelOfNeighboringNode(node)];
}

Node Particle::tail() const
{
    return head.nodeInDir(tailDir);
}

Node Particle::occupiedNodeIncidentToLabel(const int label) const
{
    if(tailDir == -1) {
        Q_ASSERT(0 <= label && label <= 5);
        return head;
    } else {
        Q_ASSERT(0 <= label && label <= 9);
        if(tailDir == posMod<6>(orientation + 0)) {
            if(label <= 2 || label >= 8) {
                return tail();
            } else {
                return head;
            }
        } else if(tailDir == posMod<6>(orientation + 1)) {
            if(label <= 3 || label >= 9) {
                return tail();
            } else {
                return head;
            }
        } else if(tailDir == posMod<6>(orientation + 2)) {
            if(label <= 2 || label >= 7) {
                return head;
            } else {
                return tail();
            }
        } else if(tailDir == posMod<6>(orientation + 3)) {
            if(label <= 2 || label >= 8) {
                return head;
            } else {
                return tail();
            }
        } else if(tailDir == posMod<6>(orientation + 4)) {
            if(label <= 3 || label >= 9) {
                return head;
            } else {
                return tail();
            }
        } else { // tailDir == posMod<6>(orientation + 0)
            if(label <= 2 || label >= 7) {
                return tail();
            } else {
                return head;
            }
        }
    }
}

Node Particle::neighboringNodeReachedViaLabel(const int label) const
{
    if(tailDir == -1) {
        Q_ASSERT(0 <= label && label <= 5);
        return head.nodeInDir(posMod<6>(orientation + label));
    } else {
        Q_ASSERT(0 <= label && label <= 9);
        if(tailDir == posMod<6>(orientation + 0)) {
            if(label == 0) {
                return tail().nodeInDir(posMod<6>(orientation + 0));
            } else if(label == 1) {
                return tail().nodeInDir(posMod<6>(orientation + 1));
            } else if(label == 2) {
                return tail().nodeInDir(posMod<6>(orientation + 2));
            } else if(label == 3) {
                return head.nodeInDir(posMod<6>(orientation + 1));
            } else if(label == 4) {
                return head.nodeInDir(posMod<6>(orientation + 2));
            } else if(label == 5) {
                return head.nodeInDir(posMod<6>(orientation + 3));
            } else if(label == 6) {
                return head.nodeInDir(posMod<6>(orientation + 4));
            } else if(label == 7) {
                return head.nodeInDir(posMod<6>(orientation + 5));
            } else if(label == 8) {
                return tail().nodeInDir(posMod<6>(orientation + 4));
            } else { // label == 9
                return tail().nodeInDir(posMod<6>(orientation + 5));
            }
        } else if(tailDir == posMod<6>(orientation + 1)) {
            if(label == 0) {
                return tail().nodeInDir(posMod<6>(orientation + 0));
            } else if(label == 1) {
                return tail().nodeInDir(posMod<6>(orientation + 1));
            } else if(label == 2) {
                return tail().nodeInDir(posMod<6>(orientation + 2));
            } else if(label == 3) {
                return tail().nodeInDir(posMod<6>(orientation + 3));
            } else if(label == 4) {
                return head.nodeInDir(posMod<6>(orientation + 2));
            } else if(label == 5) {
                return head.nodeInDir(posMod<6>(orientation + 3));
            } else if(label == 6) {
                return head.nodeInDir(posMod<6>(orientation + 4));
            } else if(label == 7) {
                return head.nodeInDir(posMod<6>(orientation + 5));
            } else if(label == 8) {
                return head.nodeInDir(posMod<6>(orientation + 0));
            } else { // label == 9
                return tail().nodeInDir(posMod<6>(orientation + 5));
            }
        } else if(tailDir == posMod<6>(orientation + 2)) {
            if(label == 0) {
                return head.nodeInDir(posMod<6>(orientation + 0));
            } else if(label == 1) {
                return head.nodeInDir(posMod<6>(orientation + 1));
            } else if(label == 2) {
                return tail().nodeInDir(posMod<6>(orientation + 0));
            } else if(label == 3) {
                return tail().nodeInDir(posMod<6>(orientation + 1));
            } else if(label == 4) {
                return tail().nodeInDir(posMod<6>(orientation + 2));
            } else if(label == 5) {
                return tail().nodeInDir(posMod<6>(orientation + 3));
            } else if(label == 6) {
                return tail().nodeInDir(posMod<6>(orientation + 4));
            } else if(label == 7) {
                return head.nodeInDir(posMod<6>(orientation + 3));
            } else if(label == 8) {
                return head.nodeInDir(posMod<6>(orientation + 4));
            } else { // label == 9
                return head.nodeInDir(posMod<6>(orientation + 5));
            }
        } else if(tailDir == posMod<6>(orientation + 3)) {
            if(label == 0) {
                return head.nodeInDir(posMod<6>(orientation + 0));
            } else if(label == 1) {
                return head.nodeInDir(posMod<6>(orientation + 1));
            } else if(label == 2) {
                return head.nodeInDir(posMod<6>(orientation + 2));
            } else if(label == 3) {
                return tail().nodeInDir(posMod<6>(orientation + 1));
            } else if(label == 4) {
                return tail().nodeInDir(posMod<6>(orientation + 2));
            } else if(label == 5) {
                return tail().nodeInDir(posMod<6>(orientation + 3));
            } else if(label == 6) {
                return tail().nodeInDir(posMod<6>(orientation + 4));
            } else if(label == 7) {
                return tail().nodeInDir(posMod<6>(orientation + 5));
            } else if(label == 8) {
                return head.nodeInDir(posMod<6>(orientation + 4));
            } else { // label == 9
                return head.nodeInDir(posMod<6>(orientation + 5));
            }
        } else if(tailDir == posMod<6>(orientation + 4)) {
            if(label == 0) {
                return head.nodeInDir(posMod<6>(orientation + 0));
            } else if(label == 1) {
                return head.nodeInDir(posMod<6>(orientation + 1));
            } else if(label == 2) {
                return head.nodeInDir(posMod<6>(orientation + 2));
            } else if(label == 3) {
                return head.nodeInDir(posMod<6>(orientation + 3));
            } else if(label == 4) {
                return tail().nodeInDir(posMod<6>(orientation + 2));
            } else if(label == 5) {
                return tail().nodeInDir(posMod<6>(orientation + 3));
            } else if(label == 6) {
                return tail().nodeInDir(posMod<6>(orientation + 4));
            } else if(label == 7) {
                return tail().nodeInDir(posMod<6>(orientation + 5));
            } else if(label == 8) {
                return tail().nodeInDir(posMod<6>(orientation + 0));
            } else { // label == 9
                return head.nodeInDir(posMod<6>(orientation + 5));
            }
        } else { // tailDir == posMod<6>(orientation + 5)
            if(label == 0) {
                return tail().nodeInDir(posMod<6>(orientation + 0));
            } else if(label == 1) {
                return tail().nodeInDir(posMod<6>(orientation + 1));
            } else if(label == 2) {
                return head.nodeInDir(posMod<6>(orientation + 0));
            } else if(label == 3) {
                return head.nodeInDir(posMod<6>(orientation + 1));
            } else if(label == 4) {
                return head.nodeInDir(posMod<6>(orientation + 2));
            } else if(label == 5) {
                return head.nodeInDir(posMod<6>(orientation + 3));
            } else if(label == 6) {
                return head.nodeInDir(posMod<6>(orientation + 4));
            } else if(label == 7) {
                return tail().nodeInDir(posMod<6>(orientation + 3));
            } else if(label == 8) {
                return tail().nodeInDir(posMod<6>(orientation + 4));
            } else { // label == 9
                return tail().nodeInDir(posMod<6>(orientation + 5));
            }
        }
    }
}

int Particle::labelOfNeighboringNode(const Node node) const
{
    int labelLimit = tailDir == -1 ? 6 : 10;
    for(int label = 0; label < labelLimit; label++) {
        if(neighboringNodeReachedViaLabel(label) == node) {
            return label;
        }
    }
    Q_ASSERT(false);
    return -1; // avoid compiler warning
}

int Particle::headColor() const
{
    return algorithm->headColor;
}

int Particle::tailColor() const
{
    return algorithm->tailColor;
}
