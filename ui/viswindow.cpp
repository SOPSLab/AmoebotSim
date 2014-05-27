#include <math.h>

#include <QImage>
#include <QMouseEvent>
#include <QOpenGLTexture>
#include <QWheelEvent>

#include "viswindow.h"

VisWindow::VisWindow(QScreen* screen) :
    GLWindow(screen),
    zoom(zoomInit)
{
}

void VisWindow::initializeGL()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);

    glClearColor(1, 1, 1, 1);
    glColor4f(1, 1, 1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);

    loadTextures();
}

void VisWindow::resizeGL()
{
    glViewport(0, 0, width(), height());
}

void VisWindow::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    setupCamera();
    drawGrid();
}

void VisWindow::loadTextures()
{
    gridTex = new QOpenGLTexture(QImage(":/textures/grid.png"));
    gridTex->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    gridTex->setMagnificationFilter(QOpenGLTexture::Linear);
    gridTex->setWrapMode(QOpenGLTexture::Repeat);

    particleTex = new QOpenGLTexture(QImage(":textures/particle.png"));
    particleTex->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    particleTex->setMagnificationFilter(QOpenGLTexture::Linear);

    for(int i = 0; i < 6; i++) {
        particleLineTex[i] = new QOpenGLTexture(QImage(QString(":/textures/particleLine%1.png").arg(i)));
        particleLineTex[i]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        particleLineTex[i]->setMagnificationFilter(QOpenGLTexture::Linear);
    }
}

void VisWindow::setupCamera()
{
    // setup view according to zoom and so that the focusPoint is in the middle
    const float halfZoomRec = 0.5f / zoom;
    view.left    = focusPos.x() - halfZoomRec * width();
    view.right   = focusPos.x() + halfZoomRec * width();
    view.bottom  = focusPos.y() + halfZoomRec * height();
    view.top     = focusPos.y() - halfZoomRec * height();

    glLoadIdentity();
    glOrtho(view.left, view.right, view.bottom, view.top, -1, 1);
}

void VisWindow::drawGrid()
{
    // Textures have to contain an integer number of pixels, but a triangle in our grid has irrational height.
    // Still, we want to tile / repeat the texture to get a background grid.
    // Hence, the part of the grid in gridTex contains a part of the grid that can be tiled
    // and this part is slightly distorted (barely noticable, but it's there).
    // The following value represents the undistorted size of the part of the grid contained
    // and has to be considered below in order to distord the texture.
    const float texHeight = 2.0f * sqrtf(0.75);

    // Coordinate sytem voodoo:
    // Calculates the texture coordinates of the corners of the shown part of the grid.
    // In that, it also reverts the distortion in the gridTex.
    Quad gridTexCoords;
    gridTexCoords.left = fmodf(view.left, 1.0f);
    gridTexCoords.right = gridTexCoords.left + view.right - view.left;
    gridTexCoords.bottom = fmodf(view.bottom, texHeight);
    gridTexCoords.top = gridTexCoords.bottom + view.top - view.bottom;
    gridTexCoords.bottom /= texHeight;
    gridTexCoords.top /= texHeight;

    // Draw screen-filling quad with gridTex according to above texture coordinates.
    gridTex->bind();
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

void VisWindow::mousePressEvent(QMouseEvent* e)
{
    if(e->buttons() & Qt::LeftButton) {
        lastMousePos = e->localPos();
        e->accept();
    }
}

void VisWindow::mouseMoveEvent(QMouseEvent* e)
{
    if(e->buttons() & Qt::LeftButton) {
        QPointF offset = lastMousePos - e->localPos();
        QPointF scaledOffset = offset / zoom;
        focusPos += QPointF(scaledOffset.x(), scaledOffset.y());
        lastMousePos = e->localPos();
        e->accept();
    }
}

void VisWindow::wheelEvent(QWheelEvent* e)
{
    zoom += e->angleDelta().y() / zoomAttenuation;
    if(zoom < zoomMin) {
        zoom = zoomMin;
    } else if(zoom > zoomMax) {
        zoom = zoomMax;
    }
    e->accept();
}
