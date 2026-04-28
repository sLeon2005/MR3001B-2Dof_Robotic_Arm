#include "EncoderPCNT.h"

#define PCNT_H_LIM 10000
#define PCNT_L_LIM -10000

EncoderPCNT::EncoderPCNT(uint8_t pinA, uint8_t pinB, int ppr, pcnt_unit_t unit) {
  _pinA = pinA;
  _pinB = pinB;
  _ppr = ppr;
  _unit = unit;
  _accumulator = 0;
}

void EncoderPCNT::begin() {
  // Canal 0: flancos de A controlado por B
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

  // Canal 1: flancos de B controlado por A (x4 completo)
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

  // Filtro de glitches (~1 µs)
  pcnt_set_filter_value(_unit, 80);
  pcnt_filter_enable(_unit);

  // Overflow: pasamos el puntero a esta instancia como arg
  pcnt_event_enable(_unit, PCNT_EVT_H_LIM);
  pcnt_event_enable(_unit, PCNT_EVT_L_LIM);
  pcnt_isr_register(_overflowHandler, this, 0, NULL);
  pcnt_intr_enable(_unit);

  pcnt_counter_pause(_unit);
  pcnt_counter_clear(_unit);
  pcnt_counter_resume(_unit);
}

int64_t EncoderPCNT::getCount() {
  int16_t hw = 0;
  pcnt_get_counter_value(_unit, &hw);
  return _accumulator + (int64_t)hw;
}

float EncoderPCNT::getDegrees() {
  return (getCount() * 360.0f) / (_ppr * 4);
}

float EncoderPCNT::getRevolutions() {
  return (float)getCount() / (_ppr * 4);
}

void EncoderPCNT::reset() {
  pcnt_counter_pause(_unit);
  pcnt_counter_clear(_unit);
  _accumulator = 0;
  pcnt_counter_resume(_unit);
}

void IRAM_ATTR EncoderPCNT::_overflowHandler(void* arg) {
  EncoderPCNT* self = (EncoderPCNT*)arg;

  uint32_t status;
  pcnt_get_event_status(self->_unit, &status);

  if (status & PCNT_EVT_H_LIM) self->_accumulator += PCNT_H_LIM;
  if (status & PCNT_EVT_L_LIM) self->_accumulator += PCNT_L_LIM;
}