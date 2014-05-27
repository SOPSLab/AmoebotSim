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

    updateTimer.setInterval(ceil(1000.0f / updateRate));
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

    const float left    = focusPos.x() - halfZoomRec * width();
    const float right   = focusPos.x() + halfZoomRec * width();
    const float bottom  = focusPos.y() + halfZoomRec * height();
    const float top     = focusPos.y() - halfZoomRec * height();

    const float width = right - left;
    const float widthSum = right + left;
    const float height = top - bottom;
    const float heightSum = top + bottom;

    // setup (transposed) orthographic projection matrix
    // see: http://en.wikipedia.org/wiki/Orthographic_projection_(geometry)
    // and: http://www.khronos.org/opengles/sdk/1.1/docs/man/glLoadMatrix.xml
    const std::array<float, 16> orthMatrix =
    {{
        2.0f / width,       0,                      0, 0,
        0,                  2.0f / height,          0, 0,
        0,                  0,                      1, 0,
        -widthSum / width,  -heightSum / height,    0, 1
    }};

    glLoadMatrixf(&orthMatrix[0]);
}

void VisWidget::drawGrid()
{
    glBindTexture(GL_TEXTURE_2D, gridTex);
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0);
    glVertex2d(-1, -1);
    glTexCoord2d(1, 0);
    glVertex2d(1, -1);
    glTexCoord2d(1, 1);
    glVertex2d(1, 1);
    glTexCoord2d(0, 1);
    glVertex2d(-1, 1);
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
