#include "MotorDriver.h"

// ===== CONSTRUCTOR =====
MotorDriver::MotorDriver(uint8_t pwmPin, uint8_t dir1, uint8_t dir2) {
  _pwmPin = pwmPin;
  _dir1 = dir1;
  _dir2 = dir2;
}

// ===== INIT =====
void MotorDriver::begin() {
  pinMode(_dir1, OUTPUT);
  pinMode(_dir2, OUTPUT);

  ledcAttach(_pwmPin, 5000, 8);
  ledcWrite(_pwmPin, 0);
}

// ===== CONFIG =====
void MotorDriver::setLimits(int pwmMin, int pwmMax) {
  _pwmMin = pwmMin;
  _pwmMax = pwmMax;
}

void MotorDriver::setMaxInput(float uMax) {
  _uMax = uMax;
}

void MotorDriver::setInverted(bool invert) {
  _inverted = invert;
}

// ===== CONTROL =====
void MotorDriver::setOutput(float u) {

  // Dirección
  if (_inverted) { 
    u = u * -1.0;
  }

  if (u >= 0) {
    digitalWrite(_dir1, HIGH);
    digitalWrite(_dir2, LOW);
  } else {
    digitalWrite(_dir1, LOW);
    digitalWrite(_dir2, HIGH);
  }

  float u_abs = fabs(u);

  float pwm = (u_abs / _uMax) * 255.0;

  if (pwm > 255) pwm = 255;

  if (pwm > 0) {
    pwm = _pwmMin + (pwm / 255.0) * (_pwmMax - _pwmMin);
  }

  ledcWrite(_pwmPin, (int)pwm);
}

// ===== STOP =====
void MotorDriver::stop() {
  ledcWrite(_pwmPin, 0);
}