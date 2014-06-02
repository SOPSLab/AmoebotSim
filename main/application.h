#ifndef APPLICATION_H
#define APPLICATION_H

#include <QGuiApplication>

class QQmlApplicationEngine;
class QThread;

class Simulator;

class Application : public QGuiApplication
{
    Q_OBJECT
public:
    explicit Application(int argc, char *argv[]);
    ~Application();

protected:
    QQmlApplicationEngine* engine;

    Simulator* sim;
    QThread* simThread;
};

#endif // APPLICATION_H
