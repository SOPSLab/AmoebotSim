#ifndef VISWINDOW_H
#define VISWINDOW_H

#include <array>

#include <QPointF>

#include "glwindow.h"

class QMouseEvent;
class QOpenGLTexture;
class QWheelEvent;

class VisWindow : public GLWindow
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
    explicit VisWindow(QScreen* screen = 0);

protected:
    virtual void initializeGL();
    virtual void resizeGL();
    virtual void paintGL();

    void loadTextures();
    void setupCamera();
    void drawGrid();
    void drawParticle(const int x, const int y);
    void drawParticle(const int x, const int y, const int dir);

    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

    inline QPointF gridToWorld(const int x, const int y);
    inline QPointF gridToWorld(const int x, const int y, const int dir);
    inline void particleQuad(const QPointF p);

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

    QPointF focusPos;
    QPointF lastMousePos;
    float zoom;
    Quad view;
};

QPointF VisWindow::gridToWorld(const int x, const int y)
{
    return QPointF(x + 0.5f * y, y * triangleHeight);
}

QPointF VisWindow::gridToWorld(const int x, const int y, const int dir)
{
    Q_ASSERT(0 <= dir && dir <= 5);

    if(dir == 0) {
        return gridToWorld(x + 1, y + 0);
    }
    if(dir == 1) {
        return gridToWorld(x + 0, y + 1);
    }
    if(dir == 2) {
        return gridToWorld(x - 1, y + 1);
    }
    if(dir == 3) {
        return gridToWorld(x - 1, y + 0);
    }
    if(dir == 4) {
        return gridToWorld(x + 0, y - 1);
    }
    if(dir == 5) {
        return gridToWorld(x + 1, y - 1);
    }

    return gridToWorld(0, 0);
}

void VisWindow::particleQuad(const QPointF p)
{
    glBegin(GL_QUADS);
    glTexCoord2d(0, 1);
    glVertex2f(p.x() - 1.0f, p.y() - 1.0f);
    glTexCoord2d(1, 1);
    glVertex2f(p.x() + 1.0f, p.y() - 1.0f);
    glTexCoord2d(1, 0);
    glVertex2f(p.x() + 1.0f, p.y() + 1.0f);
    glTexCoord2d(0, 0);
    glVertex2f(p.x() - 1.0f, p.y() + 1.0f);
    glEnd();
}

#endif // VISWINDOW_H
