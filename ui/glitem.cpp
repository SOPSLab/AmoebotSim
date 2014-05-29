#include <QOpenGLContext>
#include <QOpenGLFunctions_2_0>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QWindow>

#include "glitem.h"

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
        connect(window, &QQuickWindow::beforeRendering, this, &GLItem::delegatePaint, Qt::DirectConnection);
        connect(window, &QQuickWindow::beforeSynchronizing, this, &GLItem::sync, Qt::DirectConnection);

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
    }
}

void GLItem::delegatePaint()
{
    if(!initialized) {
        // ownership goes to context
        glfn = window()->openglContext()->versionFunctions<QOpenGLFunctions_2_0>();
        glfn->initializeOpenGLFunctions();
        initialize();
        initialized = true;
    }

    auto ratio = window()->devicePixelRatio();
    int width = int(ratio * window()->width());
    int height = int(ratio * window()->height());
    paint(width, height);
}
