/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "ui/visitem.h"

#include <cmath>

#include <QImage>
#include <QMutexLocker>
#include <QOpenGLFunctions_2_0>
#include <QQuickWindow>
#include <QRgb>

// visualisation preferences
static constexpr float targetFramesPerSecond = 60.0f;

// values derived from the preferences above
static constexpr float targetFrameDuration = 1000.0f / targetFramesPerSecond;

// height of a triangle in our equilateral triangular grid if the side length is 1
static const double triangleHeight = sqrt(3.0 / 4.0);

VisItem::VisItem(QQuickItem* parent) :
  GLItem(parent),
  translating(false) {
  setAcceptedMouseButtons(Qt::LeftButton);
  renderTimer.start(targetFrameDuration);
}

void VisItem::systemChanged(std::shared_ptr<System> _system) {
  system = _system;
}

void VisItem::focusOnCenterOfMass() {
  QMutexLocker locker(&system->mutex);
  if (system == nullptr || system->size() == 0) {
    return;
  }

  QPointF sum;
  int numMassPoints = 0;

  for (const Particle& p : *system) {
    sum = sum + nodeToWorldCoord(p.head);
    numMassPoints++;
    if (p.globalTailDir != -1) {
      sum = sum + nodeToWorldCoord(p.tail());
      numMassPoints++;
    }
  }

  for(const Object* obj: system->getObjects()) {
      sum = sum + nodeToWorldCoord(obj->_node);
      numMassPoints++;
  }

  view.setFocusPos(sum / numMassPoints);
}

void VisItem::setWindowSize(int width, int height) {
  window()->setWidth(width);
  window()->setHeight(height);
}

void VisItem::focusOn(Node node) {
  view.setFocusPos(nodeToWorldCoord(node));
}

void VisItem::setZoom(double zoom) {
  view.setZoom(zoom);
}

void VisItem::saveScreenshot(QString filePath) {
  window()->grabWindow().save(filePath);
}

void VisItem::initialize() {
  gridTex = std::unique_ptr<QOpenGLTexture>(new QOpenGLTexture(QImage(":/textures/grid.png").mirrored()));
  gridTex->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
  gridTex->setWrapMode(QOpenGLTexture::Repeat);
  gridTex->bind();
  gridTex->generateMipMaps();

  particleTex = std::unique_ptr<QOpenGLTexture>(new QOpenGLTexture(QImage(":textures/particle.png").mirrored()));
  particleTex->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
  particleTex->bind();
  particleTex->generateMipMaps();

  Q_ASSERT(window() != nullptr);
  connect(&renderTimer, &QTimer::timeout, window(), &QQuickWindow::update);
}

void VisItem::paint() {
  glfn->glUseProgram(0);

  glfn->glViewport(0, 0, width(), height());

  glfn->glDisable(GL_DEPTH_TEST);
  glfn->glDisable(GL_CULL_FACE);

  glfn->glEnable(GL_BLEND);
  glfn->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glfn->glEnable(GL_TEXTURE_2D);

  setupCamera();

  drawGrid();

  if (system != nullptr) {
    QMutexLocker locker(&system->mutex);

    drawParticles();

    drawObjects();
  }
}

void VisItem::deinitialize() {
  renderTimer.disconnect();

  particleTex = nullptr;
  gridTex = nullptr;
}

void VisItem::sizeChanged(int width, int height) {
  view.setViewportSize(width, height);
}

void VisItem::setupCamera() {
  glfn->glMatrixMode(GL_MODELVIEW);
  glfn->glLoadIdentity();
  glfn->glMatrixMode(GL_PROJECTION);
  glfn->glLoadIdentity();
  glfn->glOrtho(view.left(), view.right(), view.bottom(), view.top(), 1, -1);
}

void VisItem::drawGrid() {
  // gridTex has the height of two triangles.
  static const double gridTexHeight = 2.0 * triangleHeight;

  // Coordinate sytem voodoo:
  // Calculates the texture coordinates of the corners of the shown part of the grid.
  const double left = fmod(view.left(), 1.0);
  const double right = left + view.right() - view.left();
  const double bottom = fmod(view.bottom(), gridTexHeight) / gridTexHeight;
  const double top = bottom + (view.top() - view.bottom()) / gridTexHeight;

  // Draw screen-filling quad with gridTex according to above texture coordinates.
  gridTex->bind();
  glfn->glColor4d(1.0, 1.0, 1.0, 1.0);
  glfn->glBegin(GL_QUADS);
  glfn->glTexCoord2d(left, bottom);
  glfn->glVertex2d(view.left(), view.bottom());
  glfn->glTexCoord2d(right, bottom);
  glfn->glVertex2d(view.right(), view.bottom());
  glfn->glTexCoord2d(right, top);
  glfn->glVertex2d(view.right(), view.top());
  glfn->glTexCoord2d(left, top);
  glfn->glVertex2d(view.left(), view.top());
  glfn->glEnd();
}

void VisItem::drawParticles() {
  particleTex->bind();
  glfn->glBegin(GL_QUADS);

  // Draw particle marks, then particles, then borders, then border points.
  for (const Particle& p : *system) {
    if (view.includes(nodeToWorldCoord(p.head))) {
      drawMarks(p);
    }
  }
  for (const Particle& p : *system) {
    if (view.includes(nodeToWorldCoord(p.head))) {
      drawParticle(p);
    }
  }
  for (const Particle& p : *system) {
    if (view.includes(nodeToWorldCoord(p.head))) {
      drawBorders(p);
    }
  }
  for (const Particle& p : *system) {
    if (view.includes(nodeToWorldCoord(p.head))) {
      drawBorderPoints(p);
    }
  }

  glfn->glEnd();
}

void VisItem::drawMarks(const Particle& p) {
  // Draw head mark.
  if (p.headMarkColor() != -1) {
    auto pos = nodeToWorldCoord(p.head);
    auto color = p.headMarkColor();
    glfn->glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 180 << 23);
    drawFromParticleTex(p.headMarkGlobalDir() + 8, pos);
  }

  // Draw tail mark.
  if (p.globalTailDir != -1 && p.tailMarkColor() > -1) {
    auto pos = nodeToWorldCoord(p.tail());
    auto color = p.tailMarkColor();
    glfn->glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 180 << 23);
    drawFromParticleTex(p.tailMarkGlobalDir() + 8, pos);
  }
}

void VisItem::drawParticle(const Particle& p) {
  auto pos = nodeToWorldCoord(p.head);
  glfn->glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
  drawFromParticleTex(p.globalTailDir + 1, pos);
}

void VisItem::drawBorders(const Particle& p) {
  auto pos = nodeToWorldCoord(p.head);
  for (unsigned int i = 0; i < p.borderColors().size(); ++i) {
    if (p.borderColors().at(i) != -1) {
      auto color = p.borderColors().at(i);
      glfn->glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 180 << 23);
      drawFromParticleTex(i + 21, pos);
    }
  }
}

void VisItem::drawBorderPoints(const Particle& p) {
  auto pos = nodeToWorldCoord(p.head);
  for (unsigned int i = 0; i < p.borderPointColors().size(); ++i) {
    if (p.borderPointColors().at(i) != -1) {
      auto color = p.borderPointColors().at(i);
      glfn->glColor4i(qRed(color) << 23, qGreen(color) << 23, qBlue(color) << 23, 255 << 23);
      drawFromParticleTex(i + 15, pos);
    }
  }
}

void VisItem::drawFromParticleTex(int index, const QPointF& pos) {
  // These values are a consequence of how the particle texture was created. The
  // expression (90.0f / 96.0f) is done to handle the conversion between 90 dpi
  // and 96 dpi that Inkscape does when exporting the particle.svg as a .png.
  static constexpr int texSize = 8;
  static constexpr double invTexSize = (90.0 / 96.0) / texSize;
  static constexpr double halfQuadSideLength = 256.0 / 220.0;

  const double column = index % texSize;
  const double row = index / texSize;
  const QPointF texOffset(invTexSize * column, invTexSize * row);

  glfn->glTexCoord2d(texOffset.x(), texOffset.y());
  glfn->glVertex2d(pos.x() - halfQuadSideLength, pos.y() - halfQuadSideLength);
  glfn->glTexCoord2d(texOffset.x() + invTexSize, texOffset.y());
  glfn->glVertex2d(pos.x() + halfQuadSideLength, pos.y() - halfQuadSideLength);
  glfn->glTexCoord2d(texOffset.x() + invTexSize, texOffset.y() + invTexSize);
  glfn->glVertex2d(pos.x() + halfQuadSideLength, pos.y() + halfQuadSideLength);
  glfn->glTexCoord2d(texOffset.x(), texOffset.y() + invTexSize);
  glfn->glVertex2d(pos.x() - halfQuadSideLength, pos.y() + halfQuadSideLength);
}

void VisItem::drawObjects() {
  glfn->glBegin(GL_QUADS);

  std::deque<Object*> objects = system->getObjects();
  for(auto t : objects) {
      drawObject(*t);
  }

  glfn->glEnd();
}

void VisItem::drawObject(const Object& t) {
  auto pos = nodeToWorldCoord(t._node);
  glfn->glColor4d(0.0, 0.0, 0.0, 1.0);
  drawFromParticleTex(39, pos);
}

QPointF VisItem::nodeToWorldCoord(const Node& node) {
  return QPointF(node.x + 0.5 * node.y, node.y * triangleHeight);
}

Node VisItem::worldCoordToNode(const QPointF& worldCord) {
  const int y = std::round(worldCord.y() / triangleHeight);
  const int x = std::round(worldCord.x() - 0.5 * y);

  return Node(x, y);
}

QPointF VisItem::windowCoordToWorldCoord(const QPointF& windowCoord) {
  const double x = view.left() + (view.right() - view.left()) * windowCoord.x() / width();
  const double y = view.top() + (view.bottom() - view.top()) * windowCoord.y() / height();

  return QPointF(x, y);
}

void VisItem::mousePressEvent(QMouseEvent* e) {
  if (e->buttons() & Qt::LeftButton) {
    // Return keyboard focus to the main screen.
    forceActiveFocus();

    if (e->modifiers() & Qt::ControlModifier) {
      translating = false;
      auto clickedNode = worldCoordToNode(windowCoordToWorldCoord(e->localPos()));
      emit stepForParticleAt(clickedNode);
    } else if (e->modifiers() & Qt::AltModifier) {
      translating = false;
      auto clickedNode = worldCoordToNode(windowCoordToWorldCoord(e->localPos()));
      QString text = "";
      for (const auto& p : *system) {
        if (p.head == clickedNode || (p.isExpanded() && p.tail() == clickedNode)) {
          text = p.inspectionText();
          break;
        }
      }
      while (text.endsWith('\n')) {
        text.chop(1);
      }
      emit inspectParticle(text);
    } else {
      translating = true;
      lastMousePos = e->localPos();
    }

    if (!(e->modifiers() & Qt::AltModifier)) {
      emit inspectParticle("");
    }

    e->accept();
  }
}

void VisItem::mouseMoveEvent(QMouseEvent* e) {
  if (e->buttons() & Qt::LeftButton) {
    if (translating){
      auto mouseOffset = lastMousePos - e->localPos();
      view.modifyFocusPos(QPointF(mouseOffset.x(), -mouseOffset.y()));
      lastMousePos = e->localPos();
      e->accept();
    }
  }
}

void VisItem::wheelEvent(QWheelEvent* e) {
  QPointF mousePos(QPointF(e->posF().x(), height() - e->posF().y()));
  auto mouseAngleDelta = e->angleDelta().y();
  view.modifyZoom(mousePos, mouseAngleDelta);
  e->accept();
}
