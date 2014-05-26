#ifndef VISWIDGET_H
#define VISWIDGET_H

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
    
    void loadTexture(GLuint& tex, const QString fileName, const bool filled);

public slots:
    void tick();

protected:
    QTimer updateTimer;
};

#endif // VISWIDGET_H
