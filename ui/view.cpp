/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "ui/view.h"

#include <cmath>

#include <QMutexLocker>

// Zoom preferences.
static constexpr double zoomInit = 16.0;
static constexpr double zoomMin = 4.0;
static constexpr double zoomMax = 128.0;
static constexpr double zoomAttenuation = 500.0;

View::View() :
  mutex(QMutex::Recursive),
  _viewportWidth(900),
  _viewportHeight(600),
  _zoom(zoomInit) {}

double View::left() {
  QMutexLocker locker(&mutex);
  const double halfZoomRec = 0.5 / _zoom;
  return _focusPos.x() - halfZoomRec * _viewportWidth;
}

double View::right() {
  QMutexLocker locker(&mutex);
  const double halfZoomRec = 0.5 / _zoom;
  return _focusPos.x() + halfZoomRec * _viewportWidth;
}

double View::bottom() {
  QMutexLocker locker(&mutex);
  const double halfZoomRec = 0.5 / _zoom;
  return _focusPos.y() - halfZoomRec * _viewportHeight;
}

double View::top() {
  QMutexLocker locker(&mutex);
  const double halfZoomRec = 0.5 / _zoom;
  return _focusPos.y() + halfZoomRec * _viewportHeight;
}

bool View::includes(const QPointF& headWorldPos) {
  QMutexLocker locker(&mutex);
  static constexpr double slack = 2.0;
  return (headWorldPos.x() >= left() - slack)
         && (headWorldPos.x() <= right() + slack)
         && (headWorldPos.y() >= bottom() - slack)
         && (headWorldPos.y() <= top() + slack);
}

void View::setViewportSize(int viewportWidth, int viewportHeight) {
  QMutexLocker locker(&mutex);
  _viewportWidth = viewportWidth;
  _viewportHeight = viewportHeight;
}

void View::setFocusPos(const QPointF& focusPos) {
  QMutexLocker locker(&mutex);
  _focusPos = focusPos;
}

void View::modifyFocusPos(const QPointF& mouseOffset) {
  QMutexLocker locker(&mutex);
  QPointF scaledOffset = mouseOffset / _zoom;
  _focusPos = _focusPos + scaledOffset;
}

void View::setZoom(double zoom) {
  QMutexLocker locker(&mutex);
  _zoom = zoom;
  if (_zoom < zoomMin) {
    _zoom = zoomMin;
  } else if (_zoom > zoomMax) {
    _zoom = zoomMax;
  }
}

void View::modifyZoom(const QPointF& mousePos, double mouseAngleDelta) {
  QMutexLocker locker(&mutex);

  // Get the world space coordinate of the point under the cursor before and
  // after updating the zoom.
  const QPointF oldPos = QPointF(left(), bottom()) + mousePos / _zoom;
  setZoom(_zoom * std::exp(mouseAngleDelta / zoomAttenuation));
  const QPointF newPos = QPointF(left(), bottom()) + mousePos / _zoom;

  // Move the focus point so that the point under the cursor remains unchanged.
  _focusPos = _focusPos + oldPos - newPos;
}
