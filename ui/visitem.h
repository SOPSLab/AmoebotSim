#ifndef VISITEM_H
#define VISITEM_H

#include <cmath>

#include <QMutex>
#include <QPointF>

#include "sim/particle.h"
#include "sim/vec.h"
#include "ui/glitem.h"

class QMouseEvent;
class QOpenGLTexture;
class QWheelEvent;

class System;

class VisItem : public GLItem
{
    Q_OBJECT
protected:
    struct Quad
    {
        float left;
        float right;
        float bottom;
        float top;
    };

public:
    explicit VisItem(QQuickItem* parent = 0);

public slots:
    void updateSystem(System* _system);

protected slots:
    virtual void sync();
    virtual void initialize();
    virtual void paint();
    virtual void deinitialize();

protected:
    void setupCamera();
    void drawGrid(const Quad& view);
    void drawParticles(const Quad& view);
    void drawParticle(const Particle& p, const Quad& view);

    static Quad calculateView(QPointF focusPos, float zoom, int viewportWidth, int viewportHeight);
    static bool inView(const QPointF& headWorldPos, const Quad& view);
    static QPointF gridToWorld(Vec pos);

    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

protected:
    static constexpr float zoomMin = 10.0f;
    static constexpr float zoomMax = 200.0f;
    static constexpr float zoomInit = 50.0f;
    static constexpr float zoomAttenuation = 10.0f;

    // height of a triangle in our equilateral triangular grid if the side length is 1
    static const float triangleHeight;

    QOpenGLTexture* gridTex;
    QOpenGLTexture* particleTex;

    // these variables are used by two threads
    // variables with suffix Gui are used by the gui thread
    // and the remaining variables are used by the render thread
    // variables with the same prefix are synchronized between the threads
    QPointF lastMousePosGui;
    QPointF focusPosGui;
    QPointF focusPos;
    float zoomGui;
    float zoom;

    System* system;

    QMutex systemMutex;
};

#endif // VISITEM_H
