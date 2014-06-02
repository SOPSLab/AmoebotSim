#include <QOpenGLContext>
#include <QOpenGLFunctions_2_0>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QWindow>

#include "ui/glitem.h"

GLItem::GLItem(QQuickItem* parent) :
    QQuickItem(parent),
    glfn(nullptr),
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
        format.setVersion(2, 0);
        format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
        format.setSwapInterval(0);
        format.setRedBufferSize(8);
        format.setGreenBufferSize(8);
        format.setBlueBufferSize(8);
        format.setDepthBufferSize(24);
        format.setStencilBufferSize(8);
        window->setFormat(format);

        connect(window, &QQuickWindow::beforeSynchronizing, this, &GLItem::sync, Qt::DirectConnection);
        connect(window, &QQuickWindow::beforeRendering, this, &GLItem::delegatePaint, Qt::DirectConnection);
        connect(window, &QQuickWindow::sceneGraphAboutToStop, this, &GLItem::delegeteDeinitialize, Qt::DirectConnection);
    }
}

void GLItem::delegatePaint()
{
    if(!initialized) {
        // ownership goes to context
        glfn = window()->openglContext()->versionFunctions<QOpenGLFunctions_2_0>();
        Q_ASSERT(glfn != nullptr);
        glfn->initializeOpenGLFunctions();
        initialize();
        initialized = true;
    }

    paint();
}

void GLItem::delegeteDeinitialize()
{
    initialized = false;
    deinitialize();
    glfn = nullptr;
}

int GLItem::width() const
{
    return window()->devicePixelRatio() * window()->width();
}

int GLItem::height() const
{
    return window()->devicePixelRatio() * window()->height();
}
