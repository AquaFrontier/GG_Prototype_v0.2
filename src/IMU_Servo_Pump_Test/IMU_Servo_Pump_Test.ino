#include <Arduino_BMI270_BMM150.h>
#include <Servo.h>

// ---------- Sabertooth (peristaltic pump) ----------
const uint8_t MAX_SPEED = 127;      // Simplified Serial range
const float   PITCH_OFFSET   = 0.0; // trim if “level” isn’t 0°
const float   PITCH_DEADBAND = 2.5; // ignore tiny tilts
const float   PITCH_MAX_DEG  = 45.0;
const float   CMD_ALPHA      = 0.2; // smoothing 0–1
const int     PITCH_SIGN     = +1;  // flip to -1 to invert pump direction

float pumpFiltered = 0.0;

// ---------- Roll → Servo ----------
Servo stabilizerServo;
const int   SERVO_PIN    = 11;    // PWM output to servo
const int   SERVO_CENTER = 90;
const float ROLL_OFFSET  = 0.0;
const float ROLL_MAX_DEG = 45.0;
const bool  SERVO_REVERSE = true;

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial1.begin(9600);  // hardware UART → Sabertooth S1

  if (!IMU.begin()) {
    Serial.println("Failed to initialize BMI270/BMM150 IMU!");
    while (1);
  }

  stabilizerServo.attach(SERVO_PIN);
  stabilizerServo.write(SERVO_CENTER);

  Serial.println("IMU ready. Roll→Servo, Pitch→Pump via Serial1.");
  Serial.println("pitch_deg, roll_deg, servo_deg, pump_speed(-127..127), pump_cmd");
}

// ---------- Loop ----------
void loop() {
  float ax, ay, az;
  if (!IMU.accelerationAvailable()) {
    delay(10);
    return;
  }
  IMU.readAcceleration(ax, ay, az);

  // Compute pitch & roll (deg)
  float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
  float roll  = atan2(ay, az) * 180.0 / PI;

  // ----- Pitch → Pump -----
  float p = (pitch - PITCH_OFFSET) * PITCH_SIGN;
  if (fabs(p) < PITCH_DEADBAND) p = 0.0;
  p = constrain(p, -PITCH_MAX_DEG, PITCH_MAX_DEG);

  float targetSpeed = (p == 0.0) ? 0.0 : (p / PITCH_MAX_DEG) * MAX_SPEED;
  pumpFiltered = (1.0 - CMD_ALPHA) * pumpFiltered + CMD_ALPHA * targetSpeed;

  int speed = (int)round(pumpFiltered);
  uint8_t pumpCmd;
  if (speed > 0)      pumpCmd = constrain(speed, 1, 127);          // forward
  else if (speed < 0) pumpCmd = 128 + constrain(-speed, 1, 127);   // reverse
  else                pumpCmd = 128;                               // stop
  Serial1.write(pumpCmd);  // send to Sabertooth

  // ----- Roll → Servo -----
  float r = roll - ROLL_OFFSET;
  r = constrain(r, -ROLL_MAX_DEG, ROLL_MAX_DEG);
  int servoDeg = map((int)r, -ROLL_MAX_DEG, ROLL_MAX_DEG,
                     SERVO_REVERSE ? 180 : 0,
                     SERVO_REVERSE ? 0   : 180);
  servoDeg = constrain(servoDeg, 0, 180);
  stabilizerServo.write(servoDeg);

  // Debug output
  Serial.print(pitch, 1); Serial.print(", ");
  Serial.print(roll, 1);  Serial.print(", ");
  Serial.print(servoDeg); Serial.print(", ");
  Serial.print(speed);    Serial.print(", ");
  Serial.println(pumpCmd);

  delay(50);
}

/*
Notes:
- TX (pin 1) → Sabertooth S1.
- Flip PITCH_SIGN to invert pump direction; flip SERVO_REVERSE for servo sense.
- Use *_OFFSET to zero the installation bias.
- Increase PITCH_DEADBAND or reduce CMD_ALPHA to cut pump jitter.
*/