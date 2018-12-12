#ifndef VISITEM_H
#define VISITEM_H

#include <memory>

#include <QOpenGLTexture>
#include <QPointF>
#include <QString>
#include <QTimer>

#include "core/node.h"
#include "core/particle.h"
#include "core/system.h"
#include "ui/glitem.h"
#include "ui/view.h"
#include "core/object.h"
#include "alg/convexhull.h"

class QMouseEvent;
class QWheelEvent;

class VisItem : public GLItem
{
    Q_OBJECT
public:
    explicit VisItem(QQuickItem* parent = 0);

signals:
    void roundForParticleAt(Node node);
    void inspectParticle(QString text);

public slots:
    void systemChanged(std::shared_ptr<System> _system);
    void focusOnCenterOfMass();
    void setWindowSize(int width, int height);
    void focusOn(Node node);
    void setZoom(float zoom);
    void saveScreenshot(QString filePath);

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
    void drawFromParticleTex(int index, const QPointF& pos);
    void drawObjects();
    void drawObject(const Object& t);
    void drawConvexHull();

    static QPointF nodeToWorldCoord(const Node& node);
    static Node worldCoordToNode(const QPointF& worldCord);
    QPointF windowCoordToWorldCoord(const QPointF& windowCoord);

    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

protected:
    std::unique_ptr<QOpenGLTexture> gridTex;
    std::unique_ptr<QOpenGLTexture> particleTex;

    QTimer renderTimer;

    View view;
    QPointF lastMousePos;
    bool translating;

    std::shared_ptr<System> system;
};

#endif // VISITEM_H
