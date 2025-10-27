#include "axis.h"

Axis::Axis(int _maxX, int _maxY) {
  maxX = _maxX;
  maxY = _maxY;
}

AxisPosition Axis::getPosition(int newX, int newY) {
  x = map(newX, 0, maxX, 0, 100);
  y = map(newY, 0, maxY, 0, 100);

  if (x > 30 && x < 70 && y > 30 && y < 70) {
    return AxisPosition::AXIS_CENTER;
  }

  if (y > 30 && y < 70 && x >= 70) {
    return AxisPosition::AXIS_RIGHT;
  }

  if (x > 30 && x < 70 && y >= 70) {
    return AxisPosition::AXIS_DOWN;
  }

  if (y > 30 && y < 70 && x <= 30) {
    return AxisPosition::AXIS_LEFT;
  }

  if (x > 30 && x < 70 && y <= 30) {
    return AxisPosition::AXIS_UP;
  }

  // Error fallback
  return AxisPosition::AXIS_CENTER;
}
