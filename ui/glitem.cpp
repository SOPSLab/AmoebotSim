#include <QQuickWindow>
#include <QSurfaceFormat>

#include "ui/glitem.h"

GLItem::GLItem(QQuickItem* parent) :
    QQuickItem(parent),
    initialized(false)
{
    connect(this, &QQuickItem::windowChanged, this, &GLItem::handleWindowChanged);
}

void GLItem::handleWindowChanged(QQuickWindow* window)
{
    if(window != nullptr) {
        window->setClearBeforeRendering(false);

        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGL);
        format.setDepthBufferSize(24);
        format.setStencilBufferSize(8);
        format.setSwapInterval(0); // deactivate vertical synchronization
        window->setFormat(format);

        connect(window, &QQuickWindow::beforeSynchronizing, this, &GLItem::sync, Qt::DirectConnection);
        connect(window, &QQuickWindow::beforeRendering, this, &GLItem::delegatePaint, Qt::DirectConnection);
        connect(window, &QQuickWindow::sceneGraphAboutToStop, this, &GLItem::delegeteDeinitialize, Qt::DirectConnection);
    }
}

void GLItem::delegatePaint()
{
    if(!initialized) {
        initialize();
        initialized = true;
    }

    paint();
}

void GLItem::delegeteDeinitialize()
{
    initialized = false;
    deinitialize();
}

int GLItem::width() const
{
    return window()->devicePixelRatio() * window()->width();
}

int GLItem::height() const
{
    return window()->devicePixelRatio() * window()->height();
}
