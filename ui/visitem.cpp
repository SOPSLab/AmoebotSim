#include <cmath>

#include <QImage>
#include <QMutexLocker>
#include <QQuickWindow>

#include "sim/particle.h"
#include "sim/system.h"
#include "ui/visitem.h"

const float VisItem::triangleHeight = sqrtf(0.75f);

VisItem::VisItem(QQuickItem* parent) :
    GLItem(parent),
    zoomGui(zoomInit),
    system(nullptr)
{
    setAcceptedMouseButtons(Qt::LeftButton);
}

void VisItem::updateSystem(System* _system)
{
    QMutexLocker locker(&systemMutex);
    delete system;
    system = _system;
    window()->update();
}

void VisItem::sync()
{
    focusPos = focusPosGui;
    zoom = zoomGui;
}

void VisItem::initialize()
{
    loadTextures();
}

void VisItem::paint()
{
    glfn->glUseProgram(0);

    glfn->glViewport(0, 0, width(), height());

    glfn->glDisable(GL_DEPTH_TEST);
    glfn->glDisable(GL_CULL_FACE);

    glfn->glEnable(GL_BLEND);
    glfn->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfn->glEnable(GL_TEXTURE_2D);

    glfn->glClearColor(1, 1, 1, 1);
    glfn->glClear(GL_COLOR_BUFFER_BIT);
    glfn->glColor4f(1, 1, 1, 1);

    setupCamera();

    drawGrid();
    drawParticles();
}

void VisItem::loadTextures()
{
    gridTex = new QOpenGLTexture(QImage(":/textures/grid.png").mirrored());
    gridTex->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    gridTex->setMagnificationFilter(QOpenGLTexture::Linear);
    gridTex->setWrapMode(QOpenGLTexture::Repeat);

    particleTex = new QOpenGLTexture(QImage(":textures/particle.png").mirrored());
    particleTex->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    particleTex->setMagnificationFilter(QOpenGLTexture::Linear);

    for(int i = 0; i < 6; i++) {
        particleLineTex[i] = new QOpenGLTexture(QImage(QString(":/textures/particleLine%1.png").arg(i)).mirrored());
        particleLineTex[i]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        particleLineTex[i]->setMagnificationFilter(QOpenGLTexture::Linear);
    }
}

void VisItem::setupCamera()
{
    Quad view = calculateView(focusPos, zoom, width(), height());
    glfn->glMatrixMode(GL_MODELVIEW);
    glfn->glLoadIdentity();
    glfn->glMatrixMode(GL_PROJECTION);
    glfn->glLoadIdentity();
    glfn->glOrtho(view.left, view.right, view.bottom, view.top, 1, -1);
}

void VisItem::drawGrid()
{
    // Textures have to contain an integer number of pixels, but a triangle in our grid has irrational height.
    // Still, we want to tile / repeat the texture to get a background grid.
    // Hence, the grid texture contains a part of the grid that can be tiled
    // but is slightly distorted (barely noticable, but it's there).
    // The following value represents the undistorted height of the part of the grid contained in the texture
    // and has to be considered below in order to distord the texture.
    const float gridTexHeight = 2.0f * triangleHeight;

    Quad view = calculateView(focusPos, zoom, width(), height());

    // Coordinate sytem voodoo:
    // Calculates the texture coordinates of the corners of the shown part of the grid.
    // In that, it also reverts the distortion in the gridTex.
    Quad gridTexCoords;
    gridTexCoords.left = fmodf(view.left, 1.0f);
    gridTexCoords.right = gridTexCoords.left + view.right - view.left;
    gridTexCoords.bottom = fmodf(view.bottom, gridTexHeight);
    gridTexCoords.top = gridTexCoords.bottom + view.top - view.bottom;
    gridTexCoords.bottom /= gridTexHeight;
    gridTexCoords.top /= gridTexHeight;

    // Draw screen-filling quad with gridTex according to above texture coordinates.
    gridTex->bind();
    glfn->glBegin(GL_QUADS);
    glfn->glTexCoord2d(gridTexCoords.left, gridTexCoords.bottom);
    glfn->glVertex2f(view.left, view.bottom);
    glfn->glTexCoord2d(gridTexCoords.right, gridTexCoords.bottom);
    glfn->glVertex2f(view.right, view.bottom);
    glfn->glTexCoord2d(gridTexCoords.right, gridTexCoords.top);
    glfn->glVertex2f(view.right, view.top);
    glfn->glTexCoord2d(gridTexCoords.left, gridTexCoords.top);
    glfn->glVertex2f(view.left, view.top);
    glfn->glEnd();
}

void VisItem::drawParticles()
{
    QMutexLocker locker(&systemMutex);
    if(system != nullptr) {
        for(auto it = system->particles.begin(); it != system->particles.end(); ++it) {
            Particle& p = *it;
            if(p.tailDir == -1) {
                particleTex->bind();
                particleQuad(gridToWorld(p.headPos));
            } else {
                particleLineTex[p.tailDir]->bind();
                particleQuad(gridToWorld(p.headPos));

                particleTex->bind();
                particleQuad(gridToWorld(p.headPos, p.tailDir));
            }
        }
    }
}

VisItem::Quad VisItem::calculateView(QPointF focusPos, float zoom, int viewportWidth, int viewportHeight)
{
    // setup view according to zoom and so that the focusPoint is in the middle
    const float halfZoomRec = 0.5f / zoom;
    Quad view;
    view.left    = focusPos.x() - halfZoomRec * viewportWidth;
    view.right   = focusPos.x() + halfZoomRec * viewportWidth;
    view.bottom  = focusPos.y() - halfZoomRec * viewportHeight;
    view.top     = focusPos.y() + halfZoomRec * viewportHeight;
    return view;
}

void VisItem::mousePressEvent(QMouseEvent* e)
{
    if(e->buttons() & Qt::LeftButton) {
        lastMousePosGui = e->localPos();
        e->accept();
    }
}

void VisItem::mouseMoveEvent(QMouseEvent* e)
{
    if(e->buttons() & Qt::LeftButton) {
        QPointF offset = lastMousePosGui - e->localPos();
        QPointF scaledOffset = offset / zoomGui;
        focusPosGui += QPointF(scaledOffset.x(), -scaledOffset.y());
        lastMousePosGui = e->localPos();
        e->accept();

        if(window()) {
            window()->update();
        }
    }
}

void VisItem::wheelEvent(QWheelEvent* e)
{
    Quad view = calculateView(focusPosGui, zoomGui, width(), height());
    QPointF mousePos(QPointF(e->posF().x(), height() - e->posF().y()));

    // remember world space coordinate of the point under the cursor before changing zoom
    QPointF oldPos = QPointF(view.left, view.bottom) + mousePos / zoomGui;

    // update zoom
    zoomGui += e->angleDelta().y() / zoomAttenuation;
    if(zoomGui < zoomMin) {
        zoomGui = zoomMin;
    } else if(zoomGui > zoomMax) {
        zoomGui = zoomMax;
    }
    e->accept();

    // calculate new world space coordinate of the point under the cursor
    view = calculateView(focusPosGui, zoomGui, width(), height());
    QPointF newPos = QPointF(view.left, view.bottom) + mousePos / zoomGui;

    // move the focus point so that the point under the cursor remains unchanged
    focusPosGui = focusPosGui + oldPos - newPos;

    if(window()) {
        window()->update();
    }
}
