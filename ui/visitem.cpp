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

// values derived from the preferences above
static constexpr float targetFrameDuration = 1000.0f / targetFramesPerSecond;

// height of a triangle in our equilateral triangular grid if the side length is 1
static const float triangleHeight = sqrtf(3.0f / 4.0f);

VisItem::VisItem(QQuickItem* parent) :
    GLItem(parent),
    blinkValue(-1.0f)
{
    setAcceptedMouseButtons(Qt::LeftButton);

    renderTimer.start(targetFrameDuration);

    connect(&blinkTimer, &QTimer::timeout, [&](){blinkValue += 0.1; if(blinkValue >= 1.0) blinkValue = -1.0;});
    blinkTimer.start(15);
}

void VisItem::systemChanged(std::shared_ptr<System> _system)
{
    system = _system;
}

void VisItem::focusOnCenterOfMass()
{
    QMutexLocker locker(&system->mutex);
    if(system == nullptr || system->size() == 0) {
        return;
    }

    QPointF sum(0, 0);
    int numNodes = 0;

    for(const Particle& p : *system) {
        sum = sum + nodeToWorldCoord(p.head);
        numNodes++;
        if(p.globalTailDir != -1) {
            sum = sum + nodeToWorldCoord(p.tail());
            numNodes++;
        }
    }

    view.setFocusPos(sum / numNodes);
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

    drawGrid();

    if(system != nullptr) {
        QMutexLocker locker(&system->mutex);
        drawParticles();
//        if(system->getSystemState() == System::SystemState::Disconnected) {
//            drawDisconnectionNode();
//        }
    }
}

void VisItem::deinitialize()
{
    blinkTimer.disconnect();
    renderTimer.disconnect();

    particleTex = nullptr;
    gridTex = nullptr;
}

void VisItem::sizeChanged(int width, int height)
{
    view.setViewportSize(width, height);
}

void VisItem::setupCamera()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(view.left(), view.right(), view.bottom(), view.top(), 1, -1);
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

    // Coordinate sytem voodoo:
    // Calculates the texture coordinates of the corners of the shown part of the grid.
    // In that, it also reverts the distortion in the gridTex.
    float left = fmodf(view.left(), 1.0f);
    float right = left + view.right() - view.left();
    float bottom = fmodf(view.bottom(), gridTexHeight);
    float top = bottom + view.top() - view.bottom();
    bottom /= gridTexHeight;
    top /= gridTexHeight;

    // Draw screen-filling quad with gridTex according to above texture coordinates.
    gridTex->bind();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(left, bottom);
    glVertex2f(view.left(), view.bottom());
    glTexCoord2f(right, bottom);
    glVertex2f(view.right(), view.bottom());
    glTexCoord2f(right, top);
    glVertex2f(view.right(), view.top());
    glTexCoord2f(left, top);
    glVertex2f(view.left(), view.top());
    glEnd();
}

void VisItem::drawParticles()
{
    particleTex->bind();
    glBegin(GL_QUADS);
    for(const Particle& p : *system) {
        if(view.includes(nodeToWorldCoord(p.head))) {
            drawMarks(p);
        }
    }
    for(const Particle& p : *system) {
        if(view.includes(nodeToWorldCoord(p.head))) {
            drawParticle(p);
        }
    }
    for(const Particle& p : *system) {
        if(view.includes(nodeToWorldCoord(p.head))) {
            drawBorders(p);
        }
    }
    for(const Particle& p : *system) {
        if(view.includes(nodeToWorldCoord(p.head))) {
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
    if(p.globalTailDir != -1 && p.tailMarkColor() > -1) {
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
    drawFromParticleTex(p.globalTailDir + 1, pos);
}

void VisItem::drawBorders(const Particle& p)
{
    auto pos = nodeToWorldCoord(p.head);
    for(unsigned int i = 0; i < p.borderColors().size(); ++i) {
        if(p.borderColors().at(i) != -1) {
            QRgb color = p.borderColors().at(i);
            glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 180 << 23);
            drawFromParticleTex(i + 21, pos);
        }
    }
}

void VisItem::drawBorderPoints(const Particle& p)
{
    auto pos = nodeToWorldCoord(p.head);
    for(unsigned int i = 0; i < p.borderPointColors().size(); ++i) {
        if(p.borderPointColors().at(i) != -1) {
            QRgb color = p.borderPointColors().at(i);
            glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 255 << 23);
            drawFromParticleTex(i + 15, pos);
        }
    }
}

//void VisItem::drawDisconnectionNode()
//{
//    auto pos = nodeToWorldCoord(system->getDisconnectionNode());
//    glColor4i(255 << 23, 0, 0, (0.6f * fabsf(blinkValue) + 0.4f) * (180 << 23));
//    glBegin(GL_QUADS);
//    drawFromParticleTex(14, pos);
//    glEnd();
//}

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
    const float x = view.left() + (view.right() - view.left()) * windowCoord.x() / width();
    const float y = view.top() + (view.bottom() - view.top() ) * windowCoord.y() / height();
    return QPointF(x, y);
}

void VisItem::mousePressEvent(QMouseEvent* e)
{
    if(e->buttons() & Qt::LeftButton) {
        if(e->modifiers() & Qt::ControlModifier) {
            //Executing round for particle
            translating = false;
            auto node = worldCoordToNode(windowCoordToWorldCoord(e->localPos()));
            emit roundForParticleAt(node.x, node.y);
        } else {
            translating = true;
            lastMousePos = e->localPos();
        }
        e->accept();
    }
}

void VisItem::mouseMoveEvent(QMouseEvent* e)
{
    if(e->buttons() & Qt::LeftButton) {
        if(translating){
            QPointF offset = lastMousePos - e->localPos();
            QPointF scaledOffset = offset / view.zoom();
            view.moveFocusPos(QPointF(scaledOffset.x(), -scaledOffset.y()));
            lastMousePos = e->localPos();
            e->accept();
        }
    }
}

void VisItem::wheelEvent(QWheelEvent* e)
{
    QPointF mousePos(QPointF(e->posF().x(), height() - e->posF().y()));
    float mouseAngleDelta = e->angleDelta().y();
    view.modifyZoom(mousePos, mouseAngleDelta);
    e->accept();
}
