#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>
#include <QTimer>

#include "system.h"

class Simulator : public QObject
{
    Q_OBJECT
public:
    explicit Simulator(QObject* parent = 0);

signals:
    void updateSystem(System* _system);

public slots:
    void round();
    void start();
    void stop();

protected:
    QTimer roundTimer;

    System system;
};

#endif // SIMULATOR_H
