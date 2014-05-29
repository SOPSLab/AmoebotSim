#ifndef GLItem_H
#define GLItem_H

#include <QQuickItem>

class QOpenGLFunctions_2_0;

class GLItem : public QQuickItem
{
    Q_OBJECT
public:
    explicit GLItem(QQuickItem* parent = 0);

protected slots:
    virtual void sync() = 0;
    virtual void initialize() = 0;
    virtual void paint(const int width, const int height) = 0;

private slots:
    void handleWindowChanged(QQuickWindow* window);

    void delegatePaint();

protected:
    QOpenGLFunctions_2_0* glfn;

private:
    bool initialized;
};

#endif // GLItem_H
