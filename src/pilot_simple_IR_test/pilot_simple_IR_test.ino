// Pilot: send single-byte commands over NEC (38 kHz) using IRremote
#include <IRremote.hpp>

#define IR_SEND_PIN 3  // Nano/Uno: D3 is fine

// Command map (one byte each)
enum : uint8_t {
  CMD_NONE  = 0x00,
  CMD_LEFT  = 0x01,
  CMD_RIGHT = 0x02,
  CMD_UP    = 0x03,
  CMD_DOWN  = 0x04
};

const uint8_t sequence[] = { CMD_LEFT, CMD_RIGHT, CMD_UP, CMD_DOWN };
uint8_t idx = 0;

void setup() {
  Serial.begin(115200);
  IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK);
  Serial.println(F("Pilot simple IR test (NEC)."));
}

void loop() {
  uint8_t cmd = sequence[idx];

  // NEC wants address + command (+ repeats). We just use address 0x00.
  IrSender.sendNEC(0x00, cmd, 0); // repeats=0; library handles 38 kHz carrier

  Serial.print(F("TX cmd=0x"));
  Serial.print(cmd, HEX);
  Serial.print(F("  ("));
  switch (cmd) {
    case CMD_LEFT:  Serial.print(F("LEFT"));  break;
    case CMD_RIGHT: Serial.print(F("RIGHT")); break;
    case CMD_UP:    Serial.print(F("UP"));    break;
    case CMD_DOWN:  Serial.print(F("DOWN"));  break;
  }
  Serial.println(F(")"));

  idx = (idx + 1) % (sizeof(sequence));
  delay(300); // pace transmissions
}