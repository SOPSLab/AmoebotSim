#include <array>
#include <cmath>

#include <QGLContext>
#include <QGLFormat>
#include <QMouseEvent>
#include <QPointF>
#include <QWheelEvent>

#include "viswidget.h"

VisWidget::VisWidget(QWidget *parent)
    : QGLWidget(parent),
      zoom(zoomInit)
{
    if(!QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_2_1) {
        qFatal("OpenGL 2.1 required but not supported.");
    }

    QGLFormat format;
    format.setAlpha(true);
    format.setDepth(false);
    format.setStencil(false);
    format.setVersion(2, 1);
    setFormat(format);

    updateTimer.setInterval(ceilf(1000.0f / updateRate));
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
    updateTimer.start();
}

VisWidget::~VisWidget()
{

}

void VisWidget::logGlError(uint64_t line)
{
    GLenum error = glGetError();
    if(error == GL_INVALID_ENUM) {
        qDebug("GL_INVALID_ENUM in line %llu", line);
    } else if(error == GL_OUT_OF_MEMORY) {
        qDebug("GL_INVALID_VALUE in line %llu", line);
    } else if(error == GL_INVALID_VALUE) {
        qDebug("GL_INVALID_OPERATION in line %llu", line);
    } else if(error == GL_INVALID_OPERATION) {
        qDebug("GL_INVALID_FRAMEBUFFER_OPERATION in line %llu", line);
    } else if(error == GL_INVALID_FRAMEBUFFER_OPERATION) {
        qDebug("GL_OUT_OF_MEMORY in line %llu", line);
    }  else if(error == GL_STACK_UNDERFLOW) {
        qDebug("GL_STACK_UNDERFLOW in line %llu", line);
    } else if(error == GL_STACK_OVERFLOW) {
        qDebug("GL_STACK_OVERFLOW in line %llu", line);
    }
}

void VisWidget::initializeGL()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);

    glClearColor(1, 1, 1, 1);
    glColor4f(1, 1, 1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);

    loadTextures();

    logGlError(__LINE__);
}

void VisWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}

void VisWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    setupCamera();
    drawGrid();
}

void VisWidget::loadTextures()
{
    gridTex = loadTexture(":/textures/grid.png");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    particleTex = loadTexture(":textures/particle.png");
    for(int i = 0; i < 6; i++) {
        particleLineTex[i] = loadTexture(QString(":/textures/particleLine%1.png").arg(i));
    }
}

GLuint VisWidget::loadTexture(const QString fileName)
{
    QImage image;
    if (!image.load(fileName)) {
        qFatal("Image \"%s\" not found.", fileName.toStdString().c_str());
    }
    return bindTexture(image, GL_TEXTURE_2D, GL_RGBA, QGLContext::LinearFilteringBindOption | QGLContext::MipmapBindOption);
}

void VisWidget::setupCamera()
{
    const float halfZoomRec = 0.5f / zoom;

    view.left    = focusPos.x() - halfZoomRec * width();
    view.right   = focusPos.x() + halfZoomRec * width();
    view.bottom  = focusPos.y() + halfZoomRec * height();
    view.top     = focusPos.y() - halfZoomRec * height();

    glLoadIdentity();
    glOrtho(view.left, view.right, view.bottom, view.top, -1, 1);
}

void VisWidget::drawGrid()
{
    const float texHeight = 2.0f * sqrtf(0.75);

    Quad gridTexCoords;
    gridTexCoords.left = fmodf(view.left, 1.0f);
    gridTexCoords.right = gridTexCoords.left + view.right - view.left;
    gridTexCoords.bottom = fmodf(view.bottom, texHeight);
    gridTexCoords.top = gridTexCoords.bottom + view.top - view.bottom;
    gridTexCoords.bottom /= texHeight;
    gridTexCoords.top /= texHeight;

    glBindTexture(GL_TEXTURE_2D, gridTex);
    glBegin(GL_QUADS);
    glTexCoord2d(gridTexCoords.left, gridTexCoords.bottom);
    glVertex2f(view.left, view.bottom);
    glTexCoord2d(gridTexCoords.right, gridTexCoords.bottom);
    glVertex2f(view.right, view.bottom);
    glTexCoord2d(gridTexCoords.right, gridTexCoords.top);
    glVertex2f(view.right, view.top);
    glTexCoord2d(gridTexCoords.left, gridTexCoords.top);
    glVertex2f(view.left, view.top);
    glEnd();
}

void VisWidget::mousePressEvent(QMouseEvent* e)
{
    if(e->buttons() & Qt::LeftButton) {
        lastMousePos = e->localPos();
        e->accept();
    }
}

void VisWidget::mouseMoveEvent(QMouseEvent* e)
{
    if(e->buttons() & Qt::LeftButton) {
        QPointF offset = lastMousePos - e->localPos();
        QPointF scaledOffset = offset / zoom;
        focusPos += QPointF(scaledOffset.x(), scaledOffset.y());
        lastMousePos = e->localPos();
        e->accept();
    }
}

void VisWidget::wheelEvent(QWheelEvent* e)
{
    zoom += e->angleDelta().y() / zoomAttenuation;
    if(zoom < zoomMin) {
        zoom = zoomMin;
    } else if(zoom > zoomMax) {
        zoom = zoomMax;
    }
    e->accept();
}

void VisWidget::tick()
{

}
