struct Encoder {
  uint8_t pinA;
  uint8_t pinB;
  volatile int count;
  int ppr; // pulses per revolution (ya considerando tu método de conteo)
};

// ====== INSTANCIAS ======
Encoder enc1 = {34, 35, 0, 1600};  // ajusta PPR real

// ====== ISR GENÉRICA ======
void IRAM_ATTR handleEncoder(void* arg) {
  Encoder* enc = (Encoder*) arg;

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
    CHANGE
  );
}

// ====== CONVERSIÓN ======
float getDegrees(Encoder* enc) {
  return (enc->count * 360.0) / enc->ppr;
}

void setup() {
  Serial.begin(115200);

  initEncoder(&enc1);

  // Motor (igual que tenías)
  ledcAttach(25, 5000, 8);
  ledcWrite(25, 0);

  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);

  digitalWrite(26, HIGH);
  digitalWrite(27, LOW);
}

void loop() {
  Serial.print("Pulsos: ");
  Serial.print(enc1.count);

  Serial.print(" | Grados: ");
  Serial.println(getDegrees(&enc1));

  delay(100);
}