#include "DRV8874.h"
#include <math.h>

// ===== CONSTRUCTOR =====

DRV8874::DRV8874(uint8_t EN, uint8_t PH, uint8_t IPROPI) {
  _pinEN = EN;
  _pinPH = PH;
  _pinIPROPI = IPROPI;
}

// ===== INIT =====
void DRV8874::begin() {
  pinMode(_pinPH, OUTPUT);
  ledcAttach(_pinEN, 20000, 8);  // 20 kHz, 8 bits (0-255)
  ledcWrite(_pinEN, 0);
}

// ===== CONFIG =====
void DRV8874::setMaxInput(float maxVal) {
  _uMax = maxVal;
}

void DRV8874::setInverted(bool invert) {
  _inverted = invert;
}

void DRV8874::setPWMDeadband(int deadband) {
  _PWMdeadband = deadband;
}

// ===== CONTROL =====
void DRV8874::setOutput(float u) {
  float u_abs = fabs(u);

  // convert from u to pwm
  int pwm = (u_abs / _uMax) * _pwmMax;

  // constraining value of pwm
  if (pwm > _pwmMax) {
    pwm = _pwmMax;
  }

  // setting the pwm deadband
  if (pwm <= _PWMdeadband) {
    pwm = 0;
  }

  // check direction of input u
  bool dir;
  if (u > 0) {
    dir = 1;
  } else {
    dir = 0;
  }

  // invert the direction if motor is inverted
  if (_inverted) {
    dir = !dir;
  }

  // send pwm value
  ledcWrite(_pinEN, pwm);
  // send direction of rotation
  digitalWrite(_pinPH, dir);
}

int DRV8874::getCurrentADC() {
  return analogRead(_pinIPROPI);
}

// ===== STOP =====
void DRV8874::stop() {
  ledcWrite(_pinEN, 0);
}