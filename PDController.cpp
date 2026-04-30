#include "PDController.h"
#include <math.h>

//  constructor
PDController::PDController(float kp, float kd) {
  _kp = kp;
  _kd = kd;
}

void PDController::setGains(float kp, float kd) {
  _kp = kp;
  _kd = kd;
}

void PDController::setTolerance(float tolerance) {
  _tolerance = tolerance;
}

float PDController::compute(float setpoint, float measurement, float dt) {

  float error = setpoint - measurement;

  float u;
  if (fabs(error) >= _tolerance) {
    // Derivada de la medición (más estable)
    float diffError = (measurement - _prevMeasurement) / dt;

    // exponential filter
    _velFiltered = _alpha * _velFiltered + (1 - _alpha) * diffError;

    _prevMeasurement = measurement;

    u = (_kp * error) - (_kd * _velFiltered);
  }
  else {
    u = 0.0;
  }


  return u;
}

void PDController::reset() {
  _prevMeasurement = 0;
  _velFiltered = 0;
}