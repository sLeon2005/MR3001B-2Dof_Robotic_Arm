#ifndef PD_CONTROLLER_H
#define PD_CONTROLLER_H

class PDController {
public:
  PDController(float kp, float kd);

  void setGains(float kp, float kd);

  // Llamar cada ciclo
  float compute(float setpoint, float measurement, float dt);

  void reset();

private:
  float _kp;
  float _kd;

  float _prevMeasurement = 0.0;
  float _velFiltered = 0.0;

  float _alpha = 0.7; // filtro derivada (0–1)
};

#endif