#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

class MotorDriver {
public:
  // Constructor
  MotorDriver(uint8_t pwmPin, uint8_t dir1, uint8_t dir2);

  // Init hardware
  void begin();

  // Configuración
  void setLimits(int pwmMin, int pwmMax);
  void setMaxInput(float uMax);
  void setInverted(bool invert);

  // Control
  void setOutput(float u);
  void stop();

private:
  uint8_t _pwmPin, _dir1, _dir2;

  int _pwmMin = 160;
  int _pwmMax = 255;

  float _uMax = 100.0;

  bool _inverted = false;
};

#endif