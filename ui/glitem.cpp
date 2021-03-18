/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "ui/glitem.h"

#include <QQuickWindow>
#include <QSurfaceFormat>

GLItem::GLItem(QQuickItem* parent) :
  QQuickItem(parent),
  glfn(nullptr),
  initialized(false) {
  connect(this, &QQuickItem::windowChanged, this, &GLItem::handleWindowChanged);
}

void GLItem::handleWindowChanged(QQuickWindow* window) {
  if (window != nullptr) {
    window->setClearBeforeRendering(false);

    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapInterval(0); // deactivate vertical synchronization
    window->setFormat(format);

    connect(window, &QQuickWindow::beforeRendering, this, &GLItem::delegatePaint, Qt::DirectConnection);
    connect(window, &QQuickWindow::sceneGraphAboutToStop, this, &GLItem::delegeteDeinitialize, Qt::DirectConnection);
    connect(window, &QQuickWindow::heightChanged, this, &GLItem::delegateSizeChanged, Qt::DirectConnection);
    connect(window, &QQuickWindow::widthChanged, this, &GLItem::delegateSizeChanged, Qt::DirectConnection);
  }
}

void GLItem::delegatePaint() {
  if (!initialized) {
    // Context retains ownership.
    glfn = window()->openglContext()->versionFunctions<QOpenGLFunctions_2_0>();
    emit initialize();
    initialized = true;
  }

  emit beforeRendering();
  paint();
  emit afterRendering();
}

void GLItem::delegeteDeinitialize() {
  initialized = false;
  deinitialize();
  glfn = nullptr;
}

void GLItem::delegateSizeChanged() {
  sizeChanged(width(), height());
}

int GLItem::width() const {
  return window()->devicePixelRatio() * window()->width();
}

int GLItem::height() const {
  return window()->devicePixelRatio() * window()->height();
}
