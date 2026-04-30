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

float motor1_speed = 0;
float motor1_setpoint = 0;

float motor2_speed = 0;
float motor2_setpoint = 0;

MotorDriver motor1(motor1_EN, motor1_IN1, motor1_IN2);
EncoderPCNT enc1(enc1_A, enc1_B, 3200, PCNT_UNIT_0);
PDController motor1_PD(3.2, 0.25);  // kp, kd

DRV8874 motor2(motor2_EN, motor2_PH, motor2_IPROPI);
EncoderPCNT enc2(enc2_A, enc2_B, 2000, PCNT_UNIT_1);
PDController motor2_PD(2.0, 0.2);  // kp, kd

void setup() {
  Serial.begin(115200);

  motor1.begin();
  enc1.begin();
  enc1.setInverted(false);

  motor2.begin();
  enc2.begin();
  enc2.setInverted(false);

  motor2.setOutput(30.0);
  delay(120);
  motor2.stop();

  delay(600);
}

void loop() {

  // receive commands from Serial Monitor
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    if (cmd == '0') {
      Serial.println("Resetting all encoders");
      enc1.reset();
      enc2.reset();
    }

    else if (cmd == 'A') {
      Serial.println("Resetting encoder #1");
      enc1.reset();
    }

    else if (cmd == 'B') {
      Serial.println("Resetting encoder #2");
      enc2.reset();
    }

    else if (cmd == '1') {
      Serial.println("Calibrating encoder #1...");
      enc1.setAngle(-15.0);
      motor1_setpoint = 0.0;
      Serial.println("Encoder #1 calibrated.");
    }

    else if (cmd == '2') {
      Serial.println("Calibrating encoder #2...");
      motor2.setOutput(-20.0);

      while (motor2.getCurrentADC() < 430) {
        Serial.println("Waiting for crash...");
        delay(5);
      }

      motor2.stop();
      enc2.setAngle(-160.0);
      motor2_setpoint = -160.0;

      Serial.println("Encoder #2 calibrated.");
    }

    else if (cmd == 'D') {
      Serial.println("M2 speed = -30");
      motor1_setpoint = 90.0;
    }

    else if (cmd == 'S'){
      motor1_setpoint = enc1.getDegrees();
      motor2_setpoint = enc2.getDegrees();
    }

    else if (cmd == 'X') {
      motor1_setpoint = 60.0;
      motor2_setpoint = -120.0;
    }

    else if (cmd == 'Y') {
      Serial.println("Home Position");
      //home
      motor1_setpoint = 120.0;
      motor2_setpoint = -110.0;
    }

    else if (cmd == 'Z') {
      motor1_setpoint = 70.0;
      motor2_setpoint = -60.0;
    }
  }

  motor1_speed = motor1_PD.compute(motor1_setpoint, enc1.getDegrees(), 0.02);
  motor2_speed = motor2_PD.compute(motor2_setpoint, enc2.getDegrees(), 0.02);

  // clamp speed if off physical limits
  if (enc1.getDegrees() < 0.0) {
    motor1_speed = max(motor1_speed, 0.0f);
  }
  if (enc1.getDegrees() > 180.0) {
    motor1_speed = min(motor1_speed, 0.0f);
  }
  if (enc2.getDegrees() < -130.0) {
    motor2_speed = max(motor2_speed, 0.0f);
  }
  if (enc2.getDegrees() > 160.0) {
    motor2_speed = min(motor2_speed, 0.0f);
  }

  // set output of motors
  motor1.setOutput(motor1_speed);
  motor2.setOutput(motor2_speed);

  // debug
  /*
  Serial.print("#M1 Speed: ");
  Serial.print(motor1_speed);
  Serial.print("\t#M1 degrees: ");
  Serial.print(enc1.getDegrees(), 2);

  Serial.print("\t");

  Serial.print("#M2 Speed: ");
  Serial.print(motor2_speed);
  Serial.print("\t#M2 degrees: ");
  Serial.println(enc2.getDegrees(), 2);
  */

  Serial.print("SetpointM1:");
  Serial.print(motor1_setpoint);
  Serial.print(",");
  Serial.print("posM1:");
  Serial.print(enc1.getDegrees(), 2);
  Serial.print(",");
  Serial.print("SetpointM2:");
  Serial.print(motor2_setpoint);
  Serial.print(",");
  Serial.print("posM2:");
  Serial.println(enc2.getDegrees(), 2);

  delay(20);
}