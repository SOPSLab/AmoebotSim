/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#ifndef AMOEBOTSIM_UI_GITEM_H_
#define AMOEBOTSIM_UI_GITEM_H_

#include <QOpenGLFunctions_2_0>
#include <QQuickItem>

class GLItem : public QQuickItem {
  Q_OBJECT

 public:
  explicit GLItem(QQuickItem* parent = nullptr);

 signals:
  void beforeRendering();
  void afterRendering();

 protected slots:
  virtual void initialize() = 0;
  virtual void paint() = 0;
  virtual void deinitialize() = 0;
  virtual void sizeChanged(int width, int height) = 0;

 private slots:
  void handleWindowChanged(QQuickWindow* window);
  void delegatePaint();
  void delegeteDeinitialize();
  void delegateSizeChanged();

 protected:
  int width() const;
  int height() const;

 protected:
  QOpenGLFunctions_2_0* glfn;

 private:
  bool initialized;
};

#endif  // AMOEBOTSIM_UI_GITEM_H_
