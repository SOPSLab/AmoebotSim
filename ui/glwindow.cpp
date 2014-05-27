#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QTimer>

#include "glwindow.h"

GLWindow::GLWindow(QScreen* screen) :
    QWindow(screen),
    initialized(false)
{
    // we want an OpenGL surface
    setSurfaceType(OpenGLSurface);

    // and it should have the following format
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(2, 1);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapInterval(0); // removes unbearable mouse delay, but might cause tearing
    format.setRedBufferSize(8);
    format.setGreenBufferSize(8);
    format.setBlueBufferSize(8);
    format.setDepthBufferSize(0); // we don't need any depth values for our 2D rendering
    format.setStencilBufferSize(0);
    format.setSamples(4); // multisampling
    setFormat(format);
    create();

    context = new QOpenGLContext(this);
    context->setFormat(format);
    context->create();

    context->makeCurrent(this);
    initializeOpenGLFunctions();

    // handle resizes in OpenGL
    connect(this, &QWindow::widthChanged, this, &GLWindow::resize);
    connect(this, &QWindow::heightChanged, this, &GLWindow::resize);

    // timer that continuesly triggers painting
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &GLWindow::paint);
    updateTimer->start(1000.0f / 60.0f);
}

void GLWindow::resize()
{
    context->makeCurrent(this);
    if(!initialized) {
        initializeGL();
        initialized = true;
    }
    resizeGL();
}

void GLWindow::paint()
{
    context->makeCurrent(this);
    if(!initialized) {
        initializeGL();
        initialized = true;
    }
    paintGL();
    if(isVisible()) {
        context->swapBuffers(this);
    }
}
