#include <cmath>

#include <QImage>
#include <QMutexLocker>
#include <QOpenGLTexture>
#include <QQuickWindow>
#include <QRgb>
#include <QTime>

#include "ui/visitem.h"

// visualisation preferences
static constexpr float targetFramesPerSecond = 60.0f;

// zoom preferences
static constexpr float zoomInit = 16.0f;
static constexpr float zoomMin = 4.0f;
static constexpr float zoomMax = 128.0f;
static constexpr float zoomAttenuation = 500.0f;

// values derived from the preferences above
static constexpr float targetFrameDuration = 1000.0f / targetFramesPerSecond;

// height of a triangle in our equilateral triangular grid if the side length is 1
static const float triangleHeight = sqrtf(3.0f / 4.0f);

VisItem::VisItem(QQuickItem* parent) :
    GLItem(parent),
    zoomGui(zoomInit),
    blinkValue(-1.0f)
{
    setAcceptedMouseButtons(Qt::LeftButton);

    renderTimer.start(targetFrameDuration);

    connect(&blinkTimer, &QTimer::timeout, [&](){blinkValue += 0.1; if(blinkValue >= 1.0) blinkValue = -1.0;});
    blinkTimer.start(15);
}

void VisItem::updateSystem(std::shared_ptr<System> _system)
{
    QMutexLocker locker(&systemMutex);
    system = _system;
}

void VisItem::setZoom(float factor){
  zoomGui = factor;
  if(zoomGui < zoomMin) {
      zoomGui = zoomMin;
  } else if(zoomGui > zoomMax) {
      zoomGui = zoomMax;
  }
}

void VisItem::focusOnCenterOfMass()
{
    if(system == nullptr || system->getNumParticles() == 0) {
        return;
    }

    QPointF sum(0, 0);
    int numNodes = 0;

    for(int i = 0; i < system->getNumParticles(); i++) {
        const Particle& p = system->at(i);
        sum = sum + nodeToWorldCoord(p.head);
        numNodes++;
        if(p.tailDir != -1) {
            sum = sum + nodeToWorldCoord(p.tail());
            numNodes++;
        }
    }

    focusPosGui = sum / numNodes;
}

void VisItem::sync()
{
    focusPos = focusPosGui;
    zoom = zoomGui;
}

void VisItem::initialize()
{
    gridTex = std::make_shared<QOpenGLTexture>(QImage(":/textures/grid.png").mirrored());
    gridTex->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
    gridTex->setWrapMode(QOpenGLTexture::Repeat);
    gridTex->bind();
    gridTex->generateMipMaps();

    particleTex = std::make_shared<QOpenGLTexture>(QImage(":textures/particle.png").mirrored());
    particleTex->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
    particleTex->bind();
    particleTex->generateMipMaps();

    Q_ASSERT(window() != nullptr);
    connect(&renderTimer, &QTimer::timeout, window(), &QQuickWindow::update);
}

void VisItem::paint()
{
    glUseProgram(0);

    glViewport(0, 0, width(), height());

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);

    setupCamera();

    Quad view = calculateView(focusPos, zoom, width(), height());
    drawGrid(view);

    QMutexLocker locker(&systemMutex);
    if(system != nullptr) {
        drawParticles(view);
        if(system->getSystemState() == System::SystemState::Disconnected) {
            drawDisconnectionNode();
        }
    }
}

void VisItem::deinitialize()
{
    blinkTimer.disconnect();
    renderTimer.disconnect();

    particleTex.reset();
    gridTex.reset();
}

void VisItem::setupCamera()
{
    Quad view = calculateView(focusPos, zoom, width(), height());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(view.left, view.right, view.bottom, view.top, 1, -1);
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
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(gridTexCoords.left, gridTexCoords.bottom);
    glVertex2f(view.left, view.bottom);
    glTexCoord2f(gridTexCoords.right, gridTexCoords.bottom);
    glVertex2f(view.right, view.bottom);
    glTexCoord2f(gridTexCoords.right, gridTexCoords.top);
    glVertex2f(view.right, view.top);
    glTexCoord2f(gridTexCoords.left, gridTexCoords.top);
    glVertex2f(view.left, view.top);
    glEnd();
}

void VisItem::drawParticles(const Quad& view)
{
    particleTex->bind();
    glBegin(GL_QUADS);
    for(int i = 0; i < system->getNumParticles(); ++i) {
        const Particle& p = system->at(i);
        if(inView(nodeToWorldCoord(p.head), view)) {
            drawMarks(p);
        }
    }
    for(int i = 0; i < system->getNumParticles(); ++i) {
        const Particle& p = system->at(i);
        if(inView(nodeToWorldCoord(p.head), view)) {
            drawParticle(p);
        }
    }
    for(int i = 0; i < system->getNumParticles(); ++i) {
        const Particle& p = system->at(i);
        if(inView(nodeToWorldCoord(p.head), view)) {
            drawBorders(p);
        }
    }
    for(int i = 0; i < system->getNumParticles(); ++i) {
        const Particle& p = system->at(i);
        if(inView(nodeToWorldCoord(p.head), view)) {
            drawBorderPoints(p);
        }
    }
    glEnd();
}

void VisItem::drawMarks(const Particle& p)
{
    auto pos = nodeToWorldCoord(p.head);
    // draw mark around head
    if(p.headMarkColor() != -1) {
        QRgb color = p.headMarkColor();
        glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 180 << 23);
        drawFromParticleTex(p.headMarkDir() + 8, pos);
    }

    // draw mark around tail
    if(p.tailDir != -1 && p.tailMarkColor() > -1) {
        auto pos = nodeToWorldCoord(p.tail());
        QRgb color = p.tailMarkColor();
        glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 180 << 23);
        drawFromParticleTex(p.headMarkDir() + 8, pos);
    }
}

void VisItem::drawParticle(const Particle& p)
{
    auto pos = nodeToWorldCoord(p.head);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    drawFromParticleTex(p.tailDir + 1, pos);
}

void VisItem::drawBorders(const Particle& p)
{
    auto pos = nodeToWorldCoord(p.head);
    for(auto i = 0; i < p.borderColors().size(); ++i) {
        if(p.borderColors().at(i) != -1) {
            QRgb color = p.borderColors().at(i);
            glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 180 << 23);
            drawFromParticleTex(p.borderDir(i) + 21, pos);
        }
    }
}

void VisItem::drawBorderPoints(const Particle& p)
{
    auto pos = nodeToWorldCoord(p.head);
    for(auto i = 0; i < p.borderPointColors().size(); ++i) {
        if(p.borderPointColors().at(i) != -1) {
            QRgb color = p.borderPointColors().at(i);
            glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 255 << 23);
            drawFromParticleTex(p.borderPointDir(i) + 15, pos);
        }
    }
}

void VisItem::drawDisconnectionNode()
{
    auto pos = nodeToWorldCoord(system->getDisconnectionNode());
    glColor4i(255 << 23, 0, 0, (0.6f * fabsf(blinkValue) + 0.4f) * (180 << 23));
    glBegin(GL_QUADS);
    drawFromParticleTex(14, pos);
    glEnd();
}

void VisItem::drawFromParticleTex(const int index, const QPointF& pos)
{
    // these values are a consequence of how the particle texture was created
    static constexpr int texSize = 8;
    static constexpr float invTexSize = 1.0f / texSize;
    static constexpr float halfQuadSideLength = 256.0f / 220.0f;

    const float column = index % texSize;
    const float row = index / texSize;
    const QPointF texOffset(invTexSize * column, invTexSize * row);

    glTexCoord2f(texOffset.x(), texOffset.y());
    glVertex2f(pos.x() - halfQuadSideLength, pos.y() - halfQuadSideLength);
    glTexCoord2f(texOffset.x() + invTexSize, texOffset.y());
    glVertex2f(pos.x() + halfQuadSideLength, pos.y() - halfQuadSideLength);
    glTexCoord2f(texOffset.x() + invTexSize, texOffset.y() + invTexSize);
    glVertex2f(pos.x() + halfQuadSideLength, pos.y() + halfQuadSideLength);
    glTexCoord2f(texOffset.x(), texOffset.y() + invTexSize);
    glVertex2f(pos.x() - halfQuadSideLength, pos.y() + halfQuadSideLength);
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

Node VisItem::worldCoordToNode(QPointF worldCord)
{
    const int y = std::round(worldCord.y() / triangleHeight);
    const int x = std::round(worldCord.x() - 0.5 * y);
    return Node(x, y);
}

QPointF VisItem::windowCoordToWorldCoord(const QPointF windowCoord)
{
    auto view = calculateView(focusPosGui, zoomGui, width(), height());
    const float x = view.left + (view.right - view.left) * windowCoord.x() / width();
    const float y = view.top + (view.bottom - view.top ) * windowCoord.y() / height();
    return QPointF(x, y);
}

void VisItem::mousePressEvent(QMouseEvent* e)
{
    if(e->buttons() & Qt::LeftButton) {
        if(e->modifiers() & Qt::ControlModifier) {
            //Executing round for particle
            translatingGui = false;
            auto node = worldCoordToNode(windowCoordToWorldCoord(e->localPos()));
            emit roundForParticleAt(node.x, node.y);
        } else {
            translatingGui = true;
            lastMousePosGui = e->localPos();
        }
        e->accept();
    }
}

void VisItem::mouseMoveEvent(QMouseEvent* e)
{
    if(e->buttons() & Qt::LeftButton) {
        if(translatingGui){
            QPointF offset = lastMousePosGui - e->localPos();
            QPointF scaledOffset = offset / zoomGui;
            focusPosGui += QPointF(scaledOffset.x(), -scaledOffset.y());
            lastMousePosGui = e->localPos();
            e->accept();
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
    setZoom(zoom * std::exp(e->angleDelta().y() / zoomAttenuation));

    // calculate new world space coordinate of the point under the cursor
    view = calculateView(focusPosGui, zoomGui, width(), height());
    QPointF newPos = QPointF(view.left, view.bottom) + mousePos / zoomGui;

    // move the focus point so that the point under the cursor remains unchanged
    focusPosGui = focusPosGui + oldPos - newPos;

    e->accept();
}
