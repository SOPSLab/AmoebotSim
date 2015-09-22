#ifndef GLItem_H
#define GLItem_H

#include <QQuickItem>

class GLItem : public QQuickItem
{
    Q_OBJECT
public:
    explicit GLItem(QQuickItem* parent = 0);

protected slots:
    virtual void sync() = 0;
    virtual void initialize() = 0;
    virtual void paint() = 0;
    virtual void deinitialize() = 0;

private slots:
    void handleWindowChanged(QQuickWindow* window);
    void delegatePaint();
    void delegeteDeinitialize();

protected:
    int width() const;
    int height() const;

private:
    bool initialized;
};

#endif // GLItem_H
