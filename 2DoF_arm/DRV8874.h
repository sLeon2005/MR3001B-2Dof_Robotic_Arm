#ifndef DRV8874_MOTOR_H
#define DRV8874_MOTOR_H

#include <Arduino.h>

class DRV8874 {
public:
  // Constructor con canales definidos por el usuario
  DRV8874(uint8_t EN, uint8_t PH, uint8_t IPROPI);

  void begin();
  void setMaxInput(float maxVal);
  void setInverted(bool invert);
  void setPWMDeadband(int deadband);

  void setOutput(float u);

  int getCurrentADC();
  void stop();


private:
  uint8_t _pinEN;
  uint8_t _pinPH;
  uint8_t _pinIPROPI;

  bool _inverted = 0;

  int _PWMdeadband = 15;

  float _uMax = 100.0;
  int _pwmMax = 255;
};

#endif