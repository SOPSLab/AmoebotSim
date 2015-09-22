#ifndef VIEW
#define VIEW

#include <cmath>

#include <QMutex>
#include <QMutexLocker>
#include <QPointF>

class View
{
private:
    // zoom preferences
    static constexpr float zoomInit = 16.0f;
    static constexpr float zoomMin = 4.0f;
    static constexpr float zoomMax = 128.0f;
    static constexpr float zoomAttenuation = 500.0f;

public:
    View() :
        mutex(QMutex::Recursive),
        viewportWidth(800),
        viewportHeight(600),
        _focusPos(0, 0),
        _zoom(zoomInit)
    {
        update();
    }

    bool includes(const QPointF& headWorldPos)
    {
        QMutexLocker locker(&mutex);
        constexpr float slack = 2.0f;
        return  (headWorldPos.x() >= _left   - slack) &&
                (headWorldPos.x() <= _right  + slack) &&
                (headWorldPos.y() >= _bottom - slack) &&
                (headWorldPos.y() <= _top    + slack);
    }

    float left()
    {
        QMutexLocker locker(&mutex);
        return _left;
    }

    float right()
    {
        QMutexLocker locker(&mutex);
        return _right;
    }

    float bottom()
    {
        QMutexLocker locker(&mutex);
        return _bottom;
    }

    float top()
    {
        QMutexLocker locker(&mutex);
        return _top;
    }

    QPointF focusPos()
    {
        QMutexLocker locker(&mutex);
        return _focusPos;
    }

    float zoom()
    {
        QMutexLocker locker(&mutex);
        return _zoom;
    }

    void setViewportSize(int _viewportWidth, int _viewportHeight)
    {
        QMutexLocker locker(&mutex);
        viewportWidth = _viewportWidth;
        viewportHeight = _viewportHeight;
        update();
    }

    void setFocusPos(const QPointF& focusPos)
    {
        QMutexLocker locker(&mutex);
        _focusPos = focusPos;
        update();
    }

    void moveFocusPos(const QPointF& offset)
    {
        QMutexLocker locker(&mutex);
        _focusPos = _focusPos + offset;
        update();
    }

    void setZoom(float zoom)
    {
        QMutexLocker locker(&mutex);
        _zoom = zoom;
        if(_zoom < zoomMin) {
            _zoom = zoomMin;
        } else if(_zoom > zoomMax) {
            _zoom = zoomMax;
        }
        update();
    }

    void modifyZoom(float mouseAngleDelta)
    {
        QMutexLocker locker(&mutex);
        setZoom(_zoom * std::exp(mouseAngleDelta / zoomAttenuation));
    }

private:
    void update()
    {
        // setup view according to zoom and so that the focusPoint is in the middle
        const float halfZoomRec = 0.5f / _zoom;
        _left    = _focusPos.x() - halfZoomRec * viewportWidth;
        _right   = _focusPos.x() + halfZoomRec * viewportWidth;
        _bottom  = _focusPos.y() - halfZoomRec * viewportHeight;
        _top     = _focusPos.y() + halfZoomRec * viewportHeight;
    }

private:
    QMutex mutex;

    // these values can be set and are independent of each other
    int viewportWidth, viewportHeight;
    QPointF _focusPos;
    float _zoom;

    // these values are derived from the values above in View::update
    float _left, _right, _bottom, _top;
};

#endif // VIEW
