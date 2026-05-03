#include <ESP32Servo.h>
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

// servo
#define servoPin 13

float motor1_speed = 0;
float motor1_setpoint = 0;

float motor2_speed = 0;
float motor2_setpoint = 0;

MotorDriver motor1(motor1_EN, motor1_IN1, motor1_IN2);
EncoderPCNT enc1(enc1_A, enc1_B, 3200, PCNT_UNIT_0);
PDController motor1_PD(2.5, 0.25);  // kp, kd

DRV8874 motor2(motor2_EN, motor2_PH, motor2_IPROPI);
EncoderPCNT enc2(enc2_A, enc2_B, 2000, PCNT_UNIT_1);
PDController motor2_PD(2.0, 0.2);  // kp, kd

Servo servoGripper;

void setup() {
  Serial.begin(115200);

  ESP32PWM::allocateTimer(0);
  servoGripper.setPeriodHertz(50);
  servoGripper.attach(servoPin, 1000, 2000);
  servoGripper.write(60);

  motor1.begin();
  enc1.begin();
  enc1.setInverted(true);

  motor2.begin();
  enc2.begin();
  enc2.setInverted(true);

  delay(600);
}

unsigned long lastLog = 0;

void loop() {

  // receive commands from Serial Monitor
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    if (cmd == '0') {
      //Serial.println("Resetting all encoders");
      enc1.reset();
      enc2.reset();
    }

    else if (cmd == 'A') {
      //Serial.println("Resetting encoder #1");
      enc1.reset();
    }

    else if (cmd == 'B') {
      //Serial.println("Resetting encoder #2");
      enc2.reset();
    }

    else if (cmd == '1') {
      //Serial.println("Calibrating encoder #1...");
      enc1.setAngle(-15.30);
      motor1_setpoint = 0.0;
      //Serial.println("Encoder #1 calibrated.");
    }

    else if (cmd == '2') {
      //Serial.println("Calibrating encoder #2...");
      motor2.setOutput(-20.0);

      while (motor2.getCurrentADC() < 430) {
        //Serial.println("Waiting for crash...");
        delay(5);
      }

      motor2.stop();
      enc2.setAngle(-160.0);
      motor2_setpoint = -150.0;

      //Serial.println("Encoder #2 calibrated.");
    }

    else if (cmd == 'D') {
      //Serial.println("M2 speed = -30");
      motor1_setpoint = 90.0;
    }

    else if (cmd == 'S') {
      motor1_setpoint = enc1.getDegrees();
      motor2_setpoint = enc2.getDegrees();
    }

    // pick object (139, -25)
    else if (cmd == 'X') {
      //Serial.println("Pick Position");
      // motor1_setpoint = 60.0;
      // motor2_setpoint = -120.0;

      motor1_setpoint = theta1Deg(139.0, -23.0, a1, a4);
      motor2_setpoint = theta2Deg(139.0, -23.0, a1, a4);
    }

    // arm home position (90, 125)
    else if (cmd == 'Y') {
      //Serial.println("Home Position");
      //home
      //motor1_setpoint = 120.0;
      //motor2_setpoint = -120.0;

      motor1_setpoint = theta1Deg(90.0, 125.0, a1, a4);
      motor2_setpoint = theta2Deg(90.0, 125.0, a1, a4);
    }

    // give position (195, 120)
    else if (cmd == 'Z') {
      // motor1_setpoint = 70.0;
      // motor2_setpoint = -60.0;

      motor1_setpoint = theta1Deg(195.0, 125.0, a1, a4);
      motor2_setpoint = theta2Deg(195.0, 125.0, a1, a4);
    }

    else if (cmd == 'M') {
      //Serial.println("Opening gripper");
      servoGripper.write(60);
    }

    else if (cmd == 'N') {
      //Serial.println("Closing gripper");
      servoGripper.write(15);
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
  Serial.print("#M1\tSpeed: ");
  Serial.print(motor1_speed);
  Serial.print("   degrees: ");
  Serial.print(enc1.getDegrees(), 2);
  Serial.print("   setpoint: ");
  Serial.print(motor1_setpoint);

  Serial.print("\t");

  Serial.print("#M2\tSpeed: ");
  Serial.print(motor2_speed);
  Serial.print("   degrees: ");
  Serial.print(enc2.getDegrees(), 2);
  Serial.print("   setpoint: ");
  Serial.println(motor2_setpoint);
  */



  unsigned long now = millis();

  if (now - lastLog >= 50) {
    lastLog = now;

    float m1 = enc1.getDegrees();
    float m2 = enc2.getDegrees();

    // Formato CSV: tiempo,setpoint,medida,...
    Serial.printf("%lu,%.2f,%.2f,%.2f,%.2f\n",
                  now,
                  motor1_setpoint,
                  m1,
                  motor2_setpoint,
                  m2);
  }


  delay(20);
}