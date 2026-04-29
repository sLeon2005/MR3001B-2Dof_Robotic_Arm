#include "ik.h"
#include <math.h>

float theta1Rad(float x, float y, float l1, float l2) {
  float t2 = theta2Rad(x, y, l1, l2);

  float k1 = l1 + l2 * cos(t2);
  float k2 = l2 * sin(t2);

  return atan2(y, x) - atan2(k2, k1);
}

float theta2Rad(float x, float y, float l1, float l2) {
  float D = (x*x + y*y - l1*l1 - l2*l2) / (2.0 * l1 * l2);
  D = constrain(D, -1.0, 1.0);

  // elbow-down → signo negativo
  return atan2(-sqrt(1.0 - D*D), D);
}

float theta1Deg(float x, float y, float link1, float link2) {
  float rad = theta1Rad(x, y, link1, link2);
  return rad * 180.0 / PI;
}

float theta2Deg(float x, float y, float link1, float link2) {
  float rad = theta2Rad(x, y, link1, link2);
  return rad * 180.0 / PI;
}