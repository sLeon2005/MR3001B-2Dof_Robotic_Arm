/***

#include "INA226.h"
#include "MotorDriver.h"
#include "PDController.h"
#include "driver/pcnt.h"

// ====== CONFIGURACIÓN PCNT ======
#define PCNT_UNIT PCNT_UNIT_0
#define PCNT_H_LIM 10000
#define PCNT_L_LIM -10000

struct Encoder {
  uint8_t pinA;
  uint8_t pinB;
  int ppr;  // pulses per revolution (en modo x4 serán 4x este valor)
};

Encoder enc1 = { 34, 35, 1600 };

// Acumulador de 64 bits para rango ilimitado
static volatile int64_t encoder_accumulator = 0;

// ====== INSTANCIAS ======
MotorDriver motor1(25, 26, 27);    // pwm pin, dir1, dir2
PDController PD_motor1(1.5, 0.85);  // kp, kd

INA226 ina_motor1(0x40);

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
    .ctrl_gpio_num = enc->pinB,
    .lctrl_mode = PCNT_MODE_REVERSE,
    .hctrl_mode = PCNT_MODE_KEEP,
    .pos_mode = PCNT_COUNT_INC,
    .neg_mode = PCNT_COUNT_DEC,
    .counter_h_lim = PCNT_H_LIM,
    .counter_l_lim = PCNT_L_LIM,
    .unit = PCNT_UNIT,
    .channel = PCNT_CHANNEL_0,
  };
  pcnt_unit_config(&cfg);

  // Canal 1: cuenta flancos del canal B, controlado por A (modo x4 completo)
  pcnt_config_t cfg2 = {
    .pulse_gpio_num = enc->pinB,
    .ctrl_gpio_num = enc->pinA,
    .lctrl_mode = PCNT_MODE_KEEP,
    .hctrl_mode = PCNT_MODE_REVERSE,
    .pos_mode = PCNT_COUNT_INC,
    .neg_mode = PCNT_COUNT_DEC,
    .counter_h_lim = PCNT_H_LIM,
    .counter_l_lim = PCNT_L_LIM,
    .unit = PCNT_UNIT,
    .channel = PCNT_CHANNEL_1,
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

  Wire.begin(32, 33);

  if (!ina_motor1.begin()){
    Serial.println("Could not connect to INA226.");
  }
  ina_motor1.setMaxCurrentShunt(1, 0.11);

  motor1.begin();
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
  float degrees = (countCopy * 360.0) / (enc1.ppr * 2);

  Serial.print("Pulsos: ");
  Serial.print((long)countCopy);

  Serial.print(" | Grados: ");
  Serial.println(degrees);


  if (millis() <= 2500) {
    motor1.setOutput(PD_motor1.compute(0.0, degrees, 0.02));
    Serial.println(PD_motor1.compute(0.0, degrees, 0.02));
  }
  // else if ( millis() <= 5000){
  //   motor1.setOutput(PD_motor1.compute(45.0, degrees, 0.02));
  //   Serial.println(PD_motor1.compute(45.0, degrees, 0.02));
  // }
  else {
    motor1.setOutput(PD_motor1.compute(1000.0, degrees, 0.02));
    Serial.println(PD_motor1.compute(1000.0, degrees, 0.02));
  }
}

*/
//


/*
#include "INA226.h"
#include "BTS7960Motor.h"
#include "EncoderPCNT.h"
#include "PDController.h"

#define pinSDA 21
#define pinSCL 22

#define RPWM_motor1 25
#define LPWM_motor1 26

#define encoder1_A 34
#define encoder1_B 35

// motor 1 objects
BTS7960Motor motor1(RPWM_motor1, LPWM_motor1);
EncoderPCNT enc1(encoder1_A, encoder1_B, 3200, PCNT_UNIT_0);
INA226 ina_motor1(0x40);
PDController PD_motor1(1.5, 0.85);  // kp, kd

/*  falta de implementar (W.I.P.)
// motor 2 objects
BTS7960Motor motor2(RPWM_motor2, LPWM_motor2);
EncoderPCNT enc2(encoder2_A, encoder2_B, 3200, PCNT_UNIT_1);
INA226 ina_motor2(0x99);
PDController PD_motor2(1.5, 0.85);  // kp, kd
*/
/*
void setup() {
  // arrancar serial
  Serial.begin(115200);
  delay(500);

  // arrancar i2c
  Wire.begin(pinSDA, pinSCL);
  if (!ina_motor1.begin()) {
    Serial.println("Could not connect to motor1's INA226.");
  }
  ina_motor1.setMaxCurrentShunt(2, 0.11);
  delay(500);

  // arrancar objetos del motor 1
  motor1.begin();
  motor1.setMaxInput(100);
  motor1.setOutput(0);

  enc1.begin();
  enc1.reset();


  Serial.println(enc1.getRevolutions());

  Serial.println(1);
  motor1.setOutput(60.0);
  Serial.println(2);
  delay(2200);
  Serial.println(3);
  motor1.setOutput(0.0);
  Serial.println(4);
  delay(200);
  Serial.println(5);
}

void loop() {

  motor1.setOutput(PD_motor1.compute(0.0, enc1.getDegrees(), 0.02));

  Serial.println(enc1.getRevolutions());
}
*/

#include "INA226.h"
#include "PDController.h"
#include "EncoderPCNT.h"
#include "DRV8874.h"
#include "MotorDriver.h"
#include "ik.h"

// joint 1
#define motor1_EN 26  // PWM
#define motor1_IN1 14
#define motor1_IN2 12

#define enc1_A 18
#define enc1_B 19

#define a1 125  // 125mm

// joint 2
#define motor2_EN 25  // in1
#define motor2_PH 27  // in2
#define motor2_IPROPI 34

#define enc2_A 32
#define enc2_B 33

#define a4 (72.5 + 80.3)

MotorDriver motor1(motor1_EN, motor1_IN1, motor1_IN2);
EncoderPCNT enc1(enc1_A, enc1_B, 3200, PCNT_UNIT_0);
PDController motor1_PD(1.5, 0.85);  // kp, kd

DRV8874 motor2(motor2_EN, motor2_PH, motor2_IPROPI);
EncoderPCNT enc2(enc2_A, enc2_B, 3200, PCNT_UNIT_1);
PDController motor2_PD(1.5, 0.85);

void setup() {
  Serial.begin(115200);

  motor1.begin();
  enc1.begin();
  enc1.setInverted(true);

  Serial.print("Encoder 1: ");
  Serial.println(enc1.getDegrees());

  motor1.setOutput(95.0);
  delay(130);
  motor1.stop();

  Serial.print("Encoder 1: ");
  Serial.println(enc1.getDegrees());
  delay(600);
}

void loop() {
  Serial.println(motor1_PD.compute(0.0, enc1.getDegrees(), 0.02));
  motor1.setOutput(motor1_PD.compute(0.0, enc1.getDegrees(), 0.02));
  Serial.println(enc1.getDegrees(), 3);
  Serial.println();
  delay(20);
}