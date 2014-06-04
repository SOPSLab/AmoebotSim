#include <array>
#include <cmath>

#include <QImage>
#include <QMutexLocker>
#include <QOpenGLFunctions_2_0>
#include <QOpenGLTexture>
#include <QQuickWindow>

#include "sim/particle.h"
#include "sim/system.h"
#include "ui/visitem.h"

const float VisItem::triangleHeight = sqrtf(0.75f);

VisItem::VisItem(QQuickItem* parent) :
    GLItem(parent),
    gridTex(nullptr),
    particleTex(nullptr),
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
    gridTex = new QOpenGLTexture(QImage(":/textures/grid.png").mirrored());
    gridTex->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
    gridTex->setWrapMode(QOpenGLTexture::Repeat);
    gridTex->bind();
    gridTex->generateMipMaps();

    particleTex = new QOpenGLTexture(QImage(":textures/particle.png").mirrored());
    particleTex->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
    particleTex->bind();
    particleTex->generateMipMaps();
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

    Quad view = calculateView(focusPos, zoom, width(), height());
    drawGrid(view);
    drawParticles(view);
}

void VisItem::deinitialize()
{
    delete particleTex;
    particleTex = nullptr;

    delete gridTex;
    gridTex = nullptr;
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

void VisItem::drawGrid(const Quad& view)
{
    // Textures have to contain an integer number of pixels, but a triangle in our grid has irrational height.
    // Still, we want to tile / repeat the texture to get a background grid.
    // Hence, the grid texture contains a part of the grid that can be tiled
    // but is slightly distorted (barely noticable, but it's there).
    // The following value represents the undistorted height of the part of the grid contained in the texture
    // and has to be considered below in order to distord the texture.
    const float gridTexHeight = 2.0f * triangleHeight;

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
    glColor3f(0.8, 0.8, 0.8);
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

void VisItem::drawParticles(const Quad& view)
{
    particleTex->bind();
    glfn->glBegin(GL_QUADS);
    QMutexLocker locker(&systemMutex);
    if(system != nullptr) {
        for(int i = 0; i < system->size(); ++i) {
            drawParticle(system->at(i), view);
        }
    }
    glfn->glEnd();
}

void VisItem::drawParticle(const Particle& p, const Quad& view)
{
    // these values are a consequence of how the particle texture was created
    constexpr std::array<QPointF, 7> particleTexOffsets =
    {{
         QPointF(0.0f / 3.0f , 0.0f / 3.0f),
         QPointF(1.0f / 3.0f , 0.0f / 3.0f),
         QPointF(2.0f / 3.0f , 0.0f / 3.0f),
         QPointF(0.0f / 3.0f , 1.0f / 3.0f),
         QPointF(1.0f / 3.0f , 1.0f / 3.0f),
         QPointF(2.0f / 3.0f , 1.0f / 3.0f),
         QPointF(0.0f / 3.0f , 2.0f / 3.0f)
    }};
    constexpr float oneThird = 1.0f / 3.0f;
    constexpr float halfQuadSideLength = 256.0f / 220.0f;

    auto pos = nodeToWorldCoord(p.head);
    if(inView(pos, view)) {
        glColor3f(0, 0, 0);
        const QPointF& texOffset = particleTexOffsets[p.tailDir + 1];
        glfn->glTexCoord2f(texOffset.x(), texOffset.y());
        glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() - halfQuadSideLength);
        glfn->glTexCoord2f(texOffset.x() + oneThird, texOffset.y());
        glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() - halfQuadSideLength);
        glfn->glTexCoord2f(texOffset.x() + oneThird, texOffset.y() + oneThird);
        glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() + halfQuadSideLength);
        glfn->glTexCoord2f(texOffset.x(), texOffset.y() + oneThird);
        glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() + halfQuadSideLength);
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

bool VisItem::inView(const QPointF& headWorldPos, const Quad& view)
{
    constexpr float slack = 2.0f;
    return  (headWorldPos.x() >= view.left   - slack) &&
            (headWorldPos.x() <= view.right  + slack) &&
            (headWorldPos.y() >= view.bottom - slack) &&
            (headWorldPos.y() <= view.top    + slack);
}

QPointF VisItem::nodeToWorldCoord(Node node)
{
    return QPointF(node.x + 0.5f * node.y, node.y * triangleHeight);
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
