#include "MotorDriver.h"
#include "PDController.h"
#include "driver/pcnt.h"

// ====== CONFIGURACIÓN PCNT ======
#define PCNT_UNIT        PCNT_UNIT_0
#define PCNT_H_LIM       10000
#define PCNT_L_LIM      -10000

struct Encoder {
  uint8_t pinA;
  uint8_t pinB;
  int ppr;  // pulses per revolution (en modo x4 serán 4x este valor)
};

Encoder enc1 = { 34, 35, 1600 };

// Acumulador de 64 bits para rango ilimitado
static volatile int64_t encoder_accumulator = 0;

// ====== INSTANCIAS ======
MotorDriver motor1(25, 26, 27);  // pwm pin, dir1, dir2
PDController PD_motor1(1.5, 0.09);  // kp, kd

// ====== CALLBACK DE OVERFLOW (hardware) ======
static void IRAM_ATTR pcnt_overflow_handler(void* arg) {
  uint32_t intr_status;
  pcnt_get_event_status(PCNT_UNIT, &intr_status);

  if (intr_status & PCNT_EVT_H_LIM) encoder_accumulator += PCNT_H_LIM;
  if (intr_status & PCNT_EVT_L_LIM) encoder_accumulator += PCNT_L_LIM;
}

// ====== INIT ENCODER (PCNT) ======
void initEncoder(Encoder* enc) {
  // Canal 0: cuenta flancos del canal A, controlado por B
  pcnt_config_t cfg = {
    .pulse_gpio_num = enc->pinA,
    .ctrl_gpio_num  = enc->pinB,
    .lctrl_mode     = PCNT_MODE_REVERSE,
    .hctrl_mode     = PCNT_MODE_KEEP,
    .pos_mode       = PCNT_COUNT_INC,
    .neg_mode       = PCNT_COUNT_DEC,
    .counter_h_lim  = PCNT_H_LIM,
    .counter_l_lim  = PCNT_L_LIM,
    .unit           = PCNT_UNIT,
    .channel        = PCNT_CHANNEL_0,
  };
  pcnt_unit_config(&cfg);

  // Canal 1: cuenta flancos del canal B, controlado por A (modo x4 completo)
  pcnt_config_t cfg2 = {
    .pulse_gpio_num = enc->pinB,
    .ctrl_gpio_num  = enc->pinA,
    .lctrl_mode     = PCNT_MODE_KEEP,
    .hctrl_mode     = PCNT_MODE_REVERSE,
    .pos_mode       = PCNT_COUNT_INC,
    .neg_mode       = PCNT_COUNT_DEC,
    .counter_h_lim  = PCNT_H_LIM,
    .counter_l_lim  = PCNT_L_LIM,
    .unit           = PCNT_UNIT,
    .channel        = PCNT_CHANNEL_1,
  };
  pcnt_unit_config(&cfg2);

  // Filtro de glitches (~1 µs a 80 MHz)
  pcnt_set_filter_value(PCNT_UNIT, 80);
  pcnt_filter_enable(PCNT_UNIT);

  // Overflow handler
  pcnt_event_enable(PCNT_UNIT, PCNT_EVT_H_LIM);
  pcnt_event_enable(PCNT_UNIT, PCNT_EVT_L_LIM);
  pcnt_isr_register(pcnt_overflow_handler, NULL, 0, NULL);
  pcnt_intr_enable(PCNT_UNIT);

  pcnt_counter_pause(PCNT_UNIT);
  pcnt_counter_clear(PCNT_UNIT);
  pcnt_counter_resume(PCNT_UNIT);
}

// ====== LECTURA Y RESET ======
int64_t encoder_get_position() {
  int16_t count = 0;
  pcnt_get_counter_value(PCNT_UNIT, &count);
  return encoder_accumulator + (int64_t)count;
}

void encoder_reset() {
  pcnt_counter_pause(PCNT_UNIT);
  pcnt_counter_clear(PCNT_UNIT);
  encoder_accumulator = 0;
  pcnt_counter_resume(PCNT_UNIT);
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);

  initEncoder(&enc1);

  motor1.begin();
  motor1.stop();
  delay(250);
  motor1.setOutput(10.0);
  delay(1000);
  //motor1.setOutput(-40.0);
  delay(1000);
  motor1.stop();
}

// ====== LOOP ======
unsigned long previousMillis = 0;
const unsigned long interval = 20;  // ms

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    mainCycle();
  }
}

// ====== CICLO PRINCIPAL ======
void mainCycle() {
  int64_t countCopy = encoder_get_position();

  // ppr * 4 porque PCNT en modo x4 cuenta 4 flancos por pulso
  float degrees = (countCopy * 360.0) / (enc1.ppr * 4);

  Serial.print("Pulsos: ");
  Serial.print((long)countCopy);

  Serial.print(" | Grados: ");
  Serial.println(degrees);

  motor1.setOutput(PD_motor1.compute(0.0, degrees, 0.02));
  Serial.println(PD_motor1.compute(0.0, degrees, 0.02));
}