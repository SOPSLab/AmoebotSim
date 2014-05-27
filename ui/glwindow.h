#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QOpenGLFunctions_2_1>
#include <QWindow>

class QOpenGLContext;
class QTimer;

class GLWindow : public QWindow, public QOpenGLFunctions_2_1
{
    Q_OBJECT

public:
    explicit GLWindow(QScreen* screen = 0);

protected:
    // typical OpenGL methods to be implemented in a subclass
    virtual void initializeGL() = 0;
    virtual void resizeGL() = 0;
    virtual void paintGL() = 0;

private slots:
    // internal methods that mainly handle calling their _GL conterpart in the right environment
    void resize();
    void paint();

private:
    bool initialized;
    QOpenGLContext* context;
    QTimer* updateTimer;
};

#endif // GLWINDOW_H
