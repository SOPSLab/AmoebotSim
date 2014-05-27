#ifndef VISWIDGET_H
#define VISWIDGET_H

#include <array>

#include <QGLWidget>
#include <QTimer>

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

public slots:
    void tick();

protected:
    static constexpr int updateTimerInterval = 33;

    QTimer updateTimer;

    GLuint gridTex;
    GLuint particleTex;
    std::array<GLuint, 6> particleLineTex;
};

#endif // VISWIDGET_H
