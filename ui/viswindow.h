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

    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

protected:
    static constexpr float zoomMin = 10.0f;
    static constexpr float zoomMax = 200.0f;
    static constexpr float zoomInit = 50.0f;
    static constexpr float zoomAttenuation = 10.0f;

    QOpenGLTexture* gridTex;
    QOpenGLTexture* particleTex;
    std::array<QOpenGLTexture*, 6> particleLineTex;

    QPointF focusPos;
    QPointF lastMousePos;
    float zoom;
    Quad view;
};

#endif // VISWINDOW_H
