#include <Arduino.h>
#include "EncoderPCNT.h"

#define PCNT_H_LIM 10000
#define PCNT_L_LIM -10000

static EncoderPCNT* _instances[PCNT_UNIT_MAX] = {};

static void IRAM_ATTR _sharedOverflowHandler(void* arg) {
  for (int i = 0; i < PCNT_UNIT_MAX; i++) {
    if (_instances[i] == nullptr) continue;

    uint32_t status;
    pcnt_get_event_status((pcnt_unit_t)i, &status);

    if (status & PCNT_EVT_H_LIM) _instances[i]->_accumulator += PCNT_H_LIM;
    if (status & PCNT_EVT_L_LIM) _instances[i]->_accumulator += PCNT_L_LIM;
  }
}

EncoderPCNT::EncoderPCNT(uint8_t pinA, uint8_t pinB, int ppr, pcnt_unit_t unit) {
  _pinA = pinA;
  _pinB = pinB;
  _ppr = ppr;
  _unit = unit;
  _accumulator = 0;
}

void EncoderPCNT::begin() {

  pinMode(_pinA, INPUT_PULLUP);
  pinMode(_pinB, INPUT_PULLUP);
  delay(10);  // stabilize those input pins

  pcnt_config_t cfg = {
    .pulse_gpio_num = _pinA,
    .ctrl_gpio_num = _pinB,
    .lctrl_mode = PCNT_MODE_REVERSE,
    .hctrl_mode = PCNT_MODE_KEEP,
    .pos_mode = PCNT_COUNT_INC,
    .neg_mode = PCNT_COUNT_DEC,
    .counter_h_lim = PCNT_H_LIM,
    .counter_l_lim = PCNT_L_LIM,
    .unit = _unit,
    .channel = PCNT_CHANNEL_0,
  };
  pcnt_unit_config(&cfg);

  pcnt_config_t cfg2 = {
    .pulse_gpio_num = _pinB,
    .ctrl_gpio_num = _pinA,
    .lctrl_mode = PCNT_MODE_KEEP,
    .hctrl_mode = PCNT_MODE_REVERSE,
    .pos_mode = PCNT_COUNT_INC,
    .neg_mode = PCNT_COUNT_DEC,
    .counter_h_lim = PCNT_H_LIM,
    .counter_l_lim = PCNT_L_LIM,
    .unit = _unit,
    .channel = PCNT_CHANNEL_1,
  };
  pcnt_unit_config(&cfg2);

  pcnt_set_filter_value(_unit, 80);
  pcnt_filter_enable(_unit);

  pcnt_event_enable(_unit, PCNT_EVT_H_LIM);
  pcnt_event_enable(_unit, PCNT_EVT_L_LIM);

  // ISR compartido, se registra una sola vez
  _instances[_unit] = this;
  static bool isr_registered = false;
  if (!isr_registered) {
    pcnt_isr_register(_sharedOverflowHandler, NULL, 0, NULL);
    isr_registered = true;
  }
  pcnt_intr_enable(_unit);

  pcnt_counter_pause(_unit);
  pcnt_counter_clear(_unit);
  pcnt_counter_resume(_unit);
}

int64_t EncoderPCNT::getCount() {
  int16_t hw = 0;
  pcnt_get_counter_value(_unit, &hw);
  return (_accumulator + (int64_t)hw) * _inverted;
}

float EncoderPCNT::getDegrees() {
  return (getCount() * 360.0f) / (_ppr);
}

float EncoderPCNT::getRevolutions() {
  return (float)getCount() / (_ppr);
}

void EncoderPCNT::setInverted(bool invert) {
  if (invert) {
    _inverted = -1;
  } else {
    _inverted = 1;
  }
}

void EncoderPCNT::reset() {
  pcnt_counter_pause(_unit);
  pcnt_counter_clear(_unit);
  _accumulator = 0;
  pcnt_counter_resume(_unit);
}

void EncoderPCNT::setAngle(float degrees) {
  // Calcula cuántos counts corresponden al ángulo deseado
  int64_t targetCount = (int64_t)((degrees * _ppr) / 360.0f) * _inverted;

  // Pausa, limpia el hw, y ajusta el acumulador
  pcnt_counter_pause(_unit);
  pcnt_counter_clear(_unit);
  _accumulator = targetCount;
  pcnt_counter_resume(_unit);
}

// Definición vacía porque ya usamos el handler global
void EncoderPCNT::_overflowHandler(void* arg) {}