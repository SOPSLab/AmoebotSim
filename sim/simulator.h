#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QTimer>

#include "system.h"

class QTimer;

class Simulator : public QObject
{
    Q_OBJECT
public:
    explicit Simulator();

signals:
    void updateSystem(System* _system);

public slots:
    void init();
    void round();
    void start();
    void stop();

protected:
    QTimer* roundTimer;

    System system;
};

#endif // SIMULATOR_H
