 // Flight Controller: receive single-byte NEC commands and print result
#include <IRremote.hpp>

#define IR_RECEIVE_PIN 2

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println(F("FC simple IR test (NEC). Waiting for commands..."));
}

void loop() {
  if (!IrReceiver.decode()) return;

  // We expect NEC; IRremote still gives us a command byte even for many remotes
  auto &d = IrReceiver.decodedIRData;

  Serial.print(F("RX proto="));
  Serial.print(d.protocol);
  Serial.print(F(" addr=0x"));
  Serial.print(d.address, HEX);
  Serial.print(F(" cmd=0x"));
  Serial.print(d.command, HEX);
  Serial.print(F("  -> "));

  switch (d.command) {
    case 0x01: Serial.println(F("LEFT"));  break;
    case 0x02: Serial.println(F("RIGHT")); break;
    case 0x03: Serial.println(F("UP"));    break;
    case 0x04: Serial.println(F("DOWN"));  break;
    default:   Serial.println(F("UNKNOWN")); break;
  }

  IrReceiver.resume(); // ready for next packet
}