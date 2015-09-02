#ifndef VISITEM_H
#define VISITEM_H

#include <cmath>

#include <QMutex>
#include <QPointF>

#include "sim/node.h"
#include "sim/particle.h"
#include "ui/glitem.h"

class QMouseEvent;
class QOpenGLTexture;
class QTimer;
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
    void insertParticleAt(const int x, const int y);

public slots:
    void updateSystem(std::shared_ptr<System> _system);
    void focusOnCenterOfMass();
    void saveScreenshot(std::shared_ptr<System> _system, QString filePath = "");
    void setZoom(float factor);
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
    static constexpr float zoomMin = 4.0f;
    static constexpr float zoomMax = 128.0f;
    static constexpr float zoomInit = 16.0f;
    static constexpr float zoomAttenuation = 500.0f;

    // these values are a consequence of how the particle texture was created
    static constexpr float oneEighth = 1.0f / 8.0f;
    static constexpr float halfQuadSideLength = 256.0f / 220.0f;

    // height of a triangle in our equilateral triangular grid if the side length is 1
    static const float triangleHeight;

    std::shared_ptr<QTimer> renderTimer;

    std::shared_ptr<QOpenGLTexture> gridTex;
    std::shared_ptr<QOpenGLTexture> particleTex;

    bool tranlatingGui;
    bool addingParticles;
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

    std::shared_ptr<QTimer> blinkTimer;
    float blinkValue;
};

#endif // VISITEM_H
