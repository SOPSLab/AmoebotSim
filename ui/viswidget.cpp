#include <QGLFormat>

#include "viswidget.h"

VisWidget::VisWidget(QWidget *parent)
    : QGLWidget(parent)
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

    updateTimer.setInterval(updateTimerInterval);
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

    logGlError(__LINE__);
}

void VisWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}

void VisWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3d(1, 0, 0);
    glRotatef(0.2, 0, 0, 1);
    glBegin(GL_QUADS);
    glVertex2d(-0.5, -0.5);
    glVertex2d(0.5, -0.5);
    glVertex2d(0.5, 0.5);
    glVertex2d(-0.5, 0.5);
    glEnd();
}

void VisWidget::loadTexture(GLuint& tex, const QString fileName, const bool filled)
{
    QImage image;
    if (!image.load(fileName)) {
        qFatal("Image \"%s\" not found.", fileName.toStdString().c_str());
    }

    image = QGLWidget::convertToGLFormat(image);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    if(filled) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    logGlError(__LINE__);
}

void VisWidget::tick()
{

}
