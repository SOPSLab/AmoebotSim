#include <array>
#include <cmath>

#include <QColor>
#include <QImage>
#include <QMutexLocker>
#include <QOpenGLFunctions_2_0>
#include <QOpenGLTexture>
#include <QTime>
#include <QTimer>
#include <QQuickWindow>

#include "sim/particle.h"
#include "sim/system.h"
#include "ui/visitem.h"

const float VisItem::triangleHeight = sqrtf(0.75f);

VisItem::VisItem(QQuickItem* parent) :
    GLItem(parent),
    zoomGui(zoomInit),
    blinkValue(-1.0f)
{
    setAcceptedMouseButtons(Qt::LeftButton);

    renderTimer.start(30);

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

void VisItem::moveCameraTo(float worldX, float worldY){
  const float y = worldY*triangleHeight;
  const float x = worldX + 0.5 * worldY;
  focusPosGui = QPointF(x, y);
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

void VisItem::saveScreenshot(std::shared_ptr<System> _system, QString filePath)
{
    updateSystem(_system);
    update();

    if(filePath == "") {
        filePath = QString("amoebotsim_") + QString::number(QTime::currentTime().msecsSinceStartOfDay()) + QString(".png");
    }

    window()->grabWindow().save(filePath);
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
    glfn->glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
    glfn->glBegin(GL_QUADS);
    glfn->glTexCoord2f(gridTexCoords.left, gridTexCoords.bottom);
    glfn->glVertex2f(view.left, view.bottom);
    glfn->glTexCoord2f(gridTexCoords.right, gridTexCoords.bottom);
    glfn->glVertex2f(view.right, view.bottom);
    glfn->glTexCoord2f(gridTexCoords.right, gridTexCoords.top);
    glfn->glVertex2f(view.right, view.top);
    glfn->glTexCoord2f(gridTexCoords.left, gridTexCoords.top);
    glfn->glVertex2f(view.left, view.top);
    glfn->glEnd();
}

void VisItem::drawParticles(const Quad& view)
{
    particleTex->bind();
    glfn->glBegin(GL_QUADS);
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
    glfn->glEnd();
}

void VisItem::drawMarks(const Particle& p)
{
    auto pos = nodeToWorldCoord(p.head);
    // draw mark around head
    if(p.headMarkColor() != -1) {
        QRgb color = p.headMarkColor();
        glfn->glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 180 << 23);
        const QPointF& texOffset = indexToParticleTexPos(p.headMarkDir() + 8);
        glfn->glTexCoord2f(texOffset.x(), texOffset.y());
        glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() - halfQuadSideLength);
        glfn->glTexCoord2f(texOffset.x() + oneEighth, texOffset.y());
        glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() - halfQuadSideLength);
        glfn->glTexCoord2f(texOffset.x() + oneEighth, texOffset.y() + oneEighth);
        glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() + halfQuadSideLength);
        glfn->glTexCoord2f(texOffset.x(), texOffset.y() + oneEighth);
        glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() + halfQuadSideLength);
    }

    // draw mark around tail
    if(p.tailDir != -1 && p.tailMarkColor() > -1) {
        auto pos = nodeToWorldCoord(p.tail());
        QRgb color = p.tailMarkColor();
        glfn->glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 180 << 23);
        const QPointF& texOffset = indexToParticleTexPos(p.tailMarkDir() + 8);
        glfn->glTexCoord2f(texOffset.x(), texOffset.y());
        glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() - halfQuadSideLength);
        glfn->glTexCoord2f(texOffset.x() + oneEighth, texOffset.y());
        glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() - halfQuadSideLength);
        glfn->glTexCoord2f(texOffset.x() + oneEighth, texOffset.y() + oneEighth);
        glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() + halfQuadSideLength);
        glfn->glTexCoord2f(texOffset.x(), texOffset.y() + oneEighth);
        glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() + halfQuadSideLength);
    }
}

void VisItem::drawParticle(const Particle& p)
{
    auto pos = nodeToWorldCoord(p.head);
    glfn->glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    const QPointF& texOffset = indexToParticleTexPos(p.tailDir + 1);
    glfn->glTexCoord2f(texOffset.x(), texOffset.y());
    glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() - halfQuadSideLength);
    glfn->glTexCoord2f(texOffset.x() + oneEighth, texOffset.y());
    glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() - halfQuadSideLength);
    glfn->glTexCoord2f(texOffset.x() + oneEighth, texOffset.y() + oneEighth);
    glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() + halfQuadSideLength);
    glfn->glTexCoord2f(texOffset.x(), texOffset.y() + oneEighth);
    glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() + halfQuadSideLength);
}

void VisItem::drawBorders(const Particle& p)
{
    auto pos = nodeToWorldCoord(p.head);
    for(auto i = 0; i < p.borderColors().size(); ++i) {
        if(p.borderColors().at(i) != -1) {
            QRgb color = p.borderColors().at(i);
            glfn->glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 180 << 23);
            const QPointF& texOffset = indexToParticleTexPos(p.borderDir(i) + 21);
            glfn->glTexCoord2f(texOffset.x(), texOffset.y());
            glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() - halfQuadSideLength);
            glfn->glTexCoord2f(texOffset.x() + oneEighth, texOffset.y());
            glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() - halfQuadSideLength);
            glfn->glTexCoord2f(texOffset.x() + oneEighth, texOffset.y() + oneEighth);
            glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() + halfQuadSideLength);
            glfn->glTexCoord2f(texOffset.x(), texOffset.y() + oneEighth);
            glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() + halfQuadSideLength);
        }
    }
}

void VisItem::drawBorderPoints(const Particle& p)
{
    auto pos = nodeToWorldCoord(p.head);
    for(auto i = 0; i < p.borderPointColors().size(); ++i) {
        if(p.borderPointColors().at(i) != -1) {
            QRgb color = p.borderPointColors().at(i);
            glfn->glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 255 << 23);
            const QPointF& texOffset = indexToParticleTexPos(p.borderPointDir(i) + 15);
            glfn->glTexCoord2f(texOffset.x(), texOffset.y());
            glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() - halfQuadSideLength);
            glfn->glTexCoord2f(texOffset.x() + oneEighth, texOffset.y());
            glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() - halfQuadSideLength);
            glfn->glTexCoord2f(texOffset.x() + oneEighth, texOffset.y() + oneEighth);
            glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() + halfQuadSideLength);
            glfn->glTexCoord2f(texOffset.x(), texOffset.y() + oneEighth);
            glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() + halfQuadSideLength);
        }
    }
}

void VisItem::drawDisconnectionNode()
{
    auto pos = nodeToWorldCoord(system->getDisconnectionNode());
    glfn->glColor4i(255 << 23, 0, 0, (0.6f * fabsf(blinkValue) + 0.4f) * (180 << 23));
    const QPointF& texOffset = indexToParticleTexPos(14);
    glfn->glBegin(GL_QUADS);
    glfn->glTexCoord2f(texOffset.x(), texOffset.y());
    glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() - halfQuadSideLength);
    glfn->glTexCoord2f(texOffset.x() + oneEighth, texOffset.y());
    glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() - halfQuadSideLength);
    glfn->glTexCoord2f(texOffset.x() + oneEighth, texOffset.y() + oneEighth);
    glfn->glVertex2f(pos.x() + halfQuadSideLength, pos.y() + halfQuadSideLength);
    glfn->glTexCoord2f(texOffset.x(), texOffset.y() + oneEighth);
    glfn->glVertex2f(pos.x() - halfQuadSideLength, pos.y() + halfQuadSideLength);
    glfn->glEnd();
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

const QPointF VisItem::indexToParticleTexPos(const int index) const
{
    static const int texSize = 8;

    float column = index % texSize;
    float row = index / texSize;

    return QPointF(column / texSize, row / texSize);
}

void VisItem::mousePressEvent(QMouseEvent* e)
{
    if(e->buttons() & Qt::LeftButton) {
        if(e->modifiers() & Qt::ControlModifier) {
            //Executing round for particle
            translatingGui = addingParticles = false;
            auto node = worldCoordToNode(windowCoordToWorldCoord(e->localPos()));
            emit roundForParticleAt(node.x, node.y);
        } else if(e->modifiers() & Qt::ShiftModifier) {
            translatingGui = false;
            addingParticles = true;
            auto node = worldCoordToNode(windowCoordToWorldCoord(e->localPos()));
            emit insertParticleAt(node.x, node.y);
        } else {
            translatingGui = true;
            addingParticles = false;
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
      } else if(addingParticles){
        auto node = worldCoordToNode(windowCoordToWorldCoord(e->localPos()));
        emit insertParticleAt(node.x, node.y);
      }
      e->accept();
    }
}

void VisItem::wheelEvent(QWheelEvent* e)
{
    Quad view = calculateView(focusPosGui, zoomGui, width(), height());
    QPointF mousePos(QPointF(e->posF().x(), height() - e->posF().y()));

    // remember world space coordinate of the point under the cursor before changing zoom
    QPointF oldPos = QPointF(view.left, view.bottom) + mousePos / zoomGui;

    // update zoom
    zoomGui *= std::exp(e->angleDelta().y() / zoomAttenuation);
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
}
