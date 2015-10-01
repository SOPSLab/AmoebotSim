#ifndef VIEW
#define VIEW

#include <cmath>

#include <QMutex>
#include <QMutexLocker>
#include <QPointF>

class View
{
public:
    View();

    float left();
    float right();
    float bottom();
    float top();

    bool includes(const QPointF& headWorldPos);

    void setViewportSize(int viewportWidth, int viewportHeight);
    void setFocusPos(const QPointF& focusPos);
    void setZoom(float zoom);

    void modifyFocusPos(const QPointF& mouseOffset);
    void modifyZoom(const QPointF& mousePos, float mouseAngleDelta);

private:
    QMutex mutex;

    int _viewportWidth, _viewportHeight;
    QPointF _focusPos;
    float _zoom;
};

#endif // VIEW
