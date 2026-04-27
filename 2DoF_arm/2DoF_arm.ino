#include "MotorDriver.h"
#include "PDController.h"

struct Encoder {
  uint8_t pinA;
  uint8_t pinB;
  volatile int count;
  int ppr;  // pulses per revolution
};

// ====== INSTANCIAS ======
Encoder enc1 = { 34, 35, 0, 1600 };


MotorDriver motor1(25, 26, 27);
PDController PD_motor1(0.5, 0.02);

// ====== ISR GENÉRICA ======
void IRAM_ATTR handleEncoder(void* arg) {
  Encoder* enc = (Encoder*)arg;

  bool A = digitalRead(enc->pinA);
  bool B = digitalRead(enc->pinB);

  if (A == B) {
    enc->count++;
  } else {
    enc->count--;
  }
}

// ====== INIT ======
void initEncoder(Encoder* enc) {
  pinMode(enc->pinA, INPUT_PULLUP);
  pinMode(enc->pinB, INPUT_PULLUP);

  attachInterruptArg(
    digitalPinToInterrupt(enc->pinA),
    handleEncoder,
    enc,
    CHANGE);
}

void setup() {
  Serial.begin(115200);

  // arrancar interrupciones de encoders
  initEncoder(&enc1);

  // constructores de motores
  motor1.begin();
  motor1.setOutput(0.0);
  delay(1200);
  motor1.setOutput(10.0);
  delay(1200);
  motor1.setOutput(-50.0);
  delay(1200);
  motor1.setOutput(-100.0);
}

unsigned long previousMillis = 0;
const unsigned long interval = 20;  // ms

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    mainCycle();  // lógica periódica
  }
}

// ciclo principal de trabajo
void mainCycle() {
  int countCopy;

  // Protección contra condición de carrera (muy importante)
  noInterrupts();
  countCopy = enc1.count;
  interrupts();

  float degrees = (countCopy * 360.0) / enc1.ppr;

  Serial.print("Pulsos: ");
  Serial.print(countCopy);

  Serial.print(" | Grados: ");
  Serial.println(degrees);
}