#ifndef VISITEM_H
#define VISITEM_H

#include <QMutex>
#include <QPointF>
#include <QTimer>

#include "sim/node.h"
#include "sim/particle.h"
#include "sim/system.h"
#include "ui/glitem.h"
#include "ui/view.h"

class QMouseEvent;
class QOpenGLTexture;
class QWheelEvent;

class VisItem : public GLItem
{
    Q_OBJECT
public:
    explicit VisItem(QQuickItem* parent = 0);

signals:
    void roundForParticleAt(const int x, const int y);

public slots:
    void systemChanged(std::shared_ptr<System> _system);
    void focusOnCenterOfMass();

protected slots:
    virtual void initialize();
    virtual void paint();
    virtual void deinitialize();
    virtual void sizeChanged(int width, int height);

protected:
    void setupCamera();

    void drawGrid();
    void drawParticles();
    void drawMarks(const Particle& p);
    void drawParticle(const Particle& p);
    void drawBorders(const Particle& p);
    void drawBorderPoints(const Particle& p);
    // FIXME: Bring this back?
//    void drawDisconnectionNode();
    void drawFromParticleTex(const int index, const QPointF& pos);

    static QPointF nodeToWorldCoord(Node node);
    static Node worldCoordToNode(QPointF worldCord);
    QPointF windowCoordToWorldCoord(const QPointF windowCoord);

    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

protected:
    std::shared_ptr<System> system;

    std::shared_ptr<QOpenGLTexture> gridTex;
    std::shared_ptr<QOpenGLTexture> particleTex;

    QTimer renderTimer;
    View view;
    bool translating;
    QPointF lastMousePos;

    QTimer blinkTimer;
    float blinkValue;
};

#endif // VISITEM_H
