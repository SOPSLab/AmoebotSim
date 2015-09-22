#ifndef VISITEM_H
#define VISITEM_H

#include <QMutex>
#include <QPointF>
#include <QTimer>

#include "sim/node.h"
#include "sim/particle.h"
#include "sim/system.h"
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

signals:
    void roundForParticleAt(const int x, const int y);

public slots:
    void updateSystem(std::shared_ptr<System> _system);
    void focusOnCenterOfMass();
    void saveScreenshot(std::shared_ptr<System> _system, QString filePath = "");
    void setZoom(float _zoom);
    void moveCameraTo(float worldX, float worldY);

protected slots:
    virtual void sync();
    virtual void initialize();
    virtual void paint();
    virtual void deinitialize();

protected:
    void setupCamera();
    void drawGrid(const Quad& view);
    void drawParticles(const Quad& view);
    void drawMarks(const Particle& p);
    void drawParticle(const Particle& p);
    void drawBorders(const Particle& p);
    void drawBorderPoints(const Particle& p);
    void drawDisconnectionNode();

    static Quad calculateView(QPointF focusPos, float zoom, int viewportWidth, int viewportHeight);
    static bool inView(const QPointF& headWorldPos, const Quad& view);
    static QPointF nodeToWorldCoord(Node node);
    static Node worldCoordToNode(QPointF worldCord);
    QPointF windowCoordToWorldCoord(const QPointF windowCoord);
    const QPointF indexToParticleTexPos(const int index) const;

    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

protected:
    QTimer renderTimer;

    std::shared_ptr<QOpenGLTexture> gridTex;
    std::shared_ptr<QOpenGLTexture> particleTex;

    bool translatingGui;
    // these variables are used by two threads
    // variables with suffix Gui are used by the gui thread
    // and the remaining variables are used by the render thread
    // variables with the same prefix are synchronized between the threads
    QPointF lastMousePosGui;
    QPointF focusPosGui;
    QPointF focusPos;
    float zoomGui;
    float zoom;

    std::shared_ptr<System> system;

    QMutex systemMutex;

    QTimer blinkTimer;
    float blinkValue;
};

#endif // VISITEM_H
