/* Copyright (C) 2021 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#ifndef AMOEBOTSIM_UI_VIEW_H_
#define AMOEBOTSIM_UI_VIEW_H_

#include <QMutex>
#include <QPointF>

class View {
 public:
  View();

  double left();
  double right();
  double bottom();
  double top();

  bool includes(const QPointF& headWorldPos);

  void setViewportSize(int viewportWidth, int viewportHeight);
  void setFocusPos(const QPointF& focusPos);
  void setZoom(double zoom);

  void modifyFocusPos(const QPointF& mouseOffset);
  void modifyZoom(const QPointF& mousePos, double mouseAngleDelta);

 private:
  QMutex mutex;

  int _viewportWidth, _viewportHeight;
  QPointF _focusPos;
  double _zoom;
};

#endif  // AMOEBOTSIM_UI_VIEW_H_
