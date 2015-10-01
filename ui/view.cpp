#include "ui/view.h"

// zoom preferences
static constexpr float zoomInit = 16.0f;
static constexpr float zoomMin = 4.0f;
static constexpr float zoomMax = 128.0f;
static constexpr float zoomAttenuation = 500.0f;

View::View() :
    mutex(QMutex::Recursive),
    _viewportWidth(800),
    _viewportHeight(600),
    _zoom(zoomInit)
{

}

float View::left()
{
    QMutexLocker locker(&mutex);
    const float halfZoomRec = 0.5f / _zoom;
    return _focusPos.x() - halfZoomRec * _viewportWidth;
}

float View::right()
{
    QMutexLocker locker(&mutex);
    const float halfZoomRec = 0.5f / _zoom;
    return _focusPos.x() + halfZoomRec * _viewportWidth;
}

float View::bottom()
{
    QMutexLocker locker(&mutex);
    const float halfZoomRec = 0.5f / _zoom;
    return _focusPos.y() - halfZoomRec * _viewportHeight;
}

float View::top()
{
    QMutexLocker locker(&mutex);
    const float halfZoomRec = 0.5f / _zoom;
    return _focusPos.y() + halfZoomRec * _viewportHeight;
}

bool View::includes(const QPointF& headWorldPos)
{
    QMutexLocker locker(&mutex);
    static constexpr float slack = 2.0f;
    return  (headWorldPos.x() >= left()   - slack) &&
            (headWorldPos.x() <= right()  + slack) &&
            (headWorldPos.y() >= bottom() - slack) &&
            (headWorldPos.y() <= top()    + slack);
}

void View::setViewportSize(int viewportWidth, int viewportHeight)
{
    QMutexLocker locker(&mutex);
    _viewportWidth = viewportWidth;
    _viewportHeight = viewportHeight;
}

void View::setFocusPos(const QPointF& focusPos)
{
    QMutexLocker locker(&mutex);
    _focusPos = focusPos;
}

void View::modifyFocusPos(const QPointF& mouseOffset)
{
    QMutexLocker locker(&mutex);
    QPointF scaledOffset = mouseOffset / _zoom;
    _focusPos = _focusPos + scaledOffset;
}

void View::setZoom(float zoom)
{
    QMutexLocker locker(&mutex);
    _zoom = zoom;
    if(_zoom < zoomMin) {
        _zoom = zoomMin;
    } else if(_zoom > zoomMax) {
        _zoom = zoomMax;
    }
}

void View::modifyZoom(const QPointF& mousePos, float mouseAngleDelta)
{
    QMutexLocker locker(&mutex);

    // remember world space coordinate of the point under the cursor before changing zoom
    const QPointF oldPos = QPointF(left(), bottom()) + mousePos / _zoom;

    // update zoom
    setZoom(_zoom * std::exp(mouseAngleDelta / zoomAttenuation));

    // calculate new world space coordinate of the point under the cursor
    const QPointF newPos = QPointF(left(), bottom()) + mousePos / _zoom;

    // move the focus point so that the point under the cursor remains unchanged
    _focusPos = _focusPos + oldPos - newPos;
}
