#include "PDController.h"

//  constructor
PDController::PDController(float kp, float kd) {
  _kp = kp;
  _kd = kd;
}

void PDController::setGains(float kp, float kd) {
  _kp = kp;
  _kd = kd;
}

float PDController::compute(float setpoint, float measurement, float dt) {

  float error = setpoint - measurement;

  // Derivada de la medición (más estable)
  float diffError = (measurement - _prevMeasurement) / dt;

  // Filtro exponencial
  _velFiltered = _alpha * _velFiltered + (1 - _alpha) * diffError;

  _prevMeasurement = measurement;

  float u = (_kp * error) - (_kd * _velFiltered);

  return u;
}

void PDController::reset() {
  _prevMeasurement = 0;
  _velFiltered = 0;
}