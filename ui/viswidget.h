#ifndef VISWIDGET_H
#define VISWIDGET_H

#include <array>

#include <QGLWidget>
#include <QPointF>
#include <QTimer>

class QMouseEvent;
class QWheelEvent;

class VisWidget : public QGLWidget
{
    Q_OBJECT

public:
    explicit VisWidget(QWidget *parent = 0);
    virtual ~VisWidget();

protected:
    void logGlError(uint64_t line);

    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    void loadTextures();
    GLuint loadTexture(const QString fileName);

    void setupCamera();
    void drawGrid();

    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

public slots:
    void tick();

protected:
    static constexpr int updateTimerInterval = 33;
    static constexpr float zoomMin = 40.0f;
    static constexpr float zoomMax = 200.0f;
    static constexpr float zoomInit = 50.0f;
    static constexpr float zoomAttenuation = 10.0f;

    QTimer updateTimer;

    GLuint gridTex;
    GLuint particleTex;
    std::array<GLuint, 6> particleLineTex;

    QPointF focusPos;
    QPointF lastMousePos;
    float zoom;
};

#endif // VISWIDGET_H
