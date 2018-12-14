#ifndef AMOEBOTSIM_UI_GITEM_H_
#define AMOEBOTSIM_UI_GITEM_H_

#include <QQuickItem>

class QOpenGLFunctions_2_0;

class GLItem : public QQuickItem {
  Q_OBJECT
 public:
  explicit GLItem(QQuickItem* parent = 0);

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
