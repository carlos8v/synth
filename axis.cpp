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

  if (x >= 65 && y >= 65) {
    return AxisPosition::AXIS_UP_RIGHT;
  }

  if (x >= 65 && y <= 35) {
    return AxisPosition::AXIS_DOWN_RIGHT;
  }

  if (x <= 35 && y >= 65) {
    return AxisPosition::AXIS_UP_LEFT;
  }

  if (x <= 35 && y <= 35) {
    return AxisPosition::AXIS_DOWN_LEFT;
  }

  if (x >= 35 && x <= 65 && y > 70) {
    return AxisPosition::AXIS_UP;
  }

  if (x >= 35 && x <= 65 && y < 30) {
    return AxisPosition::AXIS_DOWN;
  }

  if (y >= 35 && y <= 65 && x > 70) {
    return AxisPosition::AXIS_RIGHT;
  }

  if (y >= 35 && y <= 65 && x < 30) {
    return AxisPosition::AXIS_LEFT;
  }

  // Error fallback
  return AxisPosition::AXIS_CENTER;
}
