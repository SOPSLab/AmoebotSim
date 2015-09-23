#include <QtGlobal>

#include "sim/particle.h"

Particle::Particle(const Node _head, const int _tailDir)
    : head(_head), tailDir(_tailDir)
{

}

Particle::~Particle()
{
}

Node Particle::tail() const
{
    return head.nodeInDir(tailDir);
}

//int Particle::headMarkColor() const
//{
//    return algorithm->headMarkColor;
//}

//int Particle::headMarkDir() const
//{
//    Q_ASSERT(-1 <= algorithm->headMarkDir && algorithm->headMarkDir <= 5);
//    if(algorithm->headMarkDir == -1) {
//        return -1;
//    } else {
//        return (orientation + algorithm->headMarkDir) % 6;
//    }
//}

//int Particle::tailMarkColor() const
//{
//    return algorithm->tailMarkColor;
//}

//int Particle::tailMarkDir() const
//{
//    Q_ASSERT(-1 <= algorithm->tailMarkDir && algorithm->tailMarkDir <= 5);
//    if(algorithm->tailMarkDir == -1) {
//        return -1;
//    } else {
//        return (orientation + algorithm->tailMarkDir) % 6;
//    }
//}

//std::array<int, 18> Particle::borderColors() const
//{
//    return algorithm->borderColors;
//}

//int Particle::borderDir(const int dir) const
//{
//    Q_ASSERT(-1 <= dir && dir <= 17);
//    if(dir == -1) {
//        return -1;
//    } else {
//        return (3 * orientation + dir) % 18;
//    }
//}

//std::array<int, 6> Particle::borderPointColors() const
//{
//    return algorithm->borderPointColors;
//}

//int Particle::borderPointDir(const int dir) const
//{
//    Q_ASSERT(-1 <= dir && dir <= 5);
//    if(dir == -1) {
//        return -1;
//    } else {
//        return (orientation + dir) % 6;
//    }
//}
