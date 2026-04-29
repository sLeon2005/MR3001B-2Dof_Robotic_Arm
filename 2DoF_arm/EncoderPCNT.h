#ifndef ENCODER_PCNT_H
#define ENCODER_PCNT_H

#include "driver/pcnt.h"
#include <stdint.h>

class EncoderPCNT {
public:
  EncoderPCNT(uint8_t pinA, uint8_t pinB, int ppr, pcnt_unit_t unit = PCNT_UNIT_0);

  void begin();

  int64_t getCount();
  float   getDegrees();
  float   getRevolutions();

  void reset();

  volatile int64_t _accumulator = 0;  // público para acceso del ISR global

private:
  uint8_t     _pinA;
  uint8_t     _pinB;
  int         _ppr;
  pcnt_unit_t _unit;

  static void _overflowHandler(void* arg);
};

#endif