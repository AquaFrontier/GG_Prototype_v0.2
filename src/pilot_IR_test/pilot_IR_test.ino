// Board: Arduino Uno
// TX: IR LED SIG -> D3, VCC->5V, GND->GND
#include <IRremote.hpp>

#define IR_SEND_PIN 3
// Command mask bits: b0=LEFT, b1=RIGHT, b2=UP, b3=DOWN
const uint8_t SEQ[] = { 0b0001, 0b0010, 0b0100, 0b1000 };
uint8_t idx = 0;
const uint8_t ADDR = 0xA5;      // our address tag
const uint8_t REPEATS = 1;      // reliability without spamming

void setup() {
  Serial.begin(115200);
  IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK);
  Serial.println(F("Pilot (Uno) NEC send. Cycling L/R/U/D"));
}

void sendMask(uint8_t mask) {
  IrSender.sendNEC(ADDR, mask, REPEATS);
  Serial.print(F("TX mask=")); Serial.print(mask, BIN);
  Serial.println();
}

void loop() {
  sendMask(SEQ[idx]);
  idx = (idx + 1) % (sizeof(SEQ));
  delay(400); // pacing helps decoding and avoids repeat confusion
}