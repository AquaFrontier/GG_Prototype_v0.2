// Board: Arduino Uno
// RX: IR receiver OUT -> D2, VCC->5V, GND->GND
#include <IRremote.hpp>
#include <Servo.h>

#define IR_RECEIVE_PIN 2
const uint8_t ADDR = 0xA5;

// --- Optional servos (uncomment pins to enable actuation) ---
const int SERVO_YAW_PIN = 9;     // yaw (L/R)
const int SERVO_VALVE_PIN = 10;  // bladder valve (Up/Down)
Servo servoYaw, servoValve;
bool servosAttached = false;

void attachServos() {
  if (servosAttached) return;
  servoYaw.attach(SERVO_YAW_PIN);
  servoValve.attach(SERVO_VALVE_PIN);
  servoYaw.write(90);     // neutral
  servoValve.write(90);   // neutral/closed midpoint
  servosAttached = true;
}

void handleMask(uint8_t mask) {
  bool LEFT  = mask & 0b0001;
  bool RIGHT = mask & 0b0010;
  bool UP    = mask & 0b0100;
  bool DOWN  = mask & 0b1000;

  Serial.print(F("RX mask=")); Serial.print(mask, BIN);
  Serial.print(F("  L:")); Serial.print(LEFT);
  Serial.print(F(" R:")); Serial.print(RIGHT);
  Serial.print(F(" U:")); Serial.print(UP);
  Serial.print(F(" D:")); Serial.println(DOWN);

  // --- Optional crude mapping (enable if servos connected) ---
  if (!servosAttached) return;  // comment this line to always drive servos

  int yaw   = 90 + (int)(RIGHT - LEFT) * 30;  // ±30° about center
  int valve = 90 + (int)(UP - DOWN) * 30;     // ±30° about center
  servoYaw.write(constrain(yaw, 45, 135));
  servoValve.write(constrain(valve, 45, 135));
}

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println(F("FC (Uno) NEC receive. Waiting..."));

  // attachServos(); // <- uncomment when hardware is connected
}

void loop() {
  if (!IrReceiver.decode()) return;

  auto &d = IrReceiver.decodedIRData;

  // Accept only clean NEC 32-bit, non-repeat, matching address.
  if (d.protocol == NEC &&
      d.numberOfBits == 32 &&
      !(d.flags & IRDATA_FLAGS_IS_REPEAT) &&
      (uint8_t)d.address == ADDR) {
    handleMask((uint8_t)d.command);
  }
  // else: ignore noise/repeats silently

  IrReceiver.resume(); // ready for next frame
}