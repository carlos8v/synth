#ifndef AXIS_H
#define AXIS_H

#ifdef ARDUINO
#include "Arduino.h"
#else
inline int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#endif

typedef enum {
  AXIS_CENTER = 0,
  AXIS_UP = 1,
  AXIS_RIGHT = 2,
  AXIS_DOWN = 3,
  AXIS_LEFT = 4,
} AxisPosition;

class Axis {
 public:
  int x;
  int y;

  Axis(int maxX, int maxY);
  AxisPosition getPosition(int newX, int newY);

 private:
  int maxX;
  int maxY;
};

#endif
