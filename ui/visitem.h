#ifndef VISITEM_H
#define VISITEM_H

#include <array>

#include <QOpenGLFunctions_2_0>
#include <QOpenGLTexture>
#include <QPointF>

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

protected:
    void loadTextures();
    void setupCamera();
    void drawGrid();
    void drawParticles();

    static Quad calculateView(QPointF focusPos, float zoom, int viewportWidth, int viewportHeight);
    inline static QPointF gridToWorld(Vec pos, const int dir = -1);
    inline void particleQuad(const QPointF p);

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
    std::array<QOpenGLTexture*, 6> particleLineTex;

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
};

QPointF VisItem::gridToWorld(Vec pos, const int dir)
{
    if(dir != -1) {
        pos = pos.vecInDir(dir);
    }
    return QPointF(pos.x + 0.5f * pos.y, pos.y * triangleHeight);
}

void VisItem::particleQuad(const QPointF p)
{
    glfn->glBegin(GL_QUADS);
    glfn->glTexCoord2d(0, 0);
    glfn->glVertex2f(p.x() - 1.0f, p.y() - 1.0f);
    glfn->glTexCoord2d(1, 0);
    glfn->glVertex2f(p.x() + 1.0f, p.y() - 1.0f);
    glfn->glTexCoord2d(1, 1);
    glfn->glVertex2f(p.x() + 1.0f, p.y() + 1.0f);
    glfn->glTexCoord2d(0, 1);
    glfn->glVertex2f(p.x() - 1.0f, p.y() + 1.0f);
    glfn->glEnd();
}

#endif // VISITEM_H
