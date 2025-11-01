#include <Arduino_BMI270_BMM150.h>
#include <Servo.h>
Servo stabilizerServo;
const int SERVO_PIN = 11;   // PWM pin connected to servo signal line
const int SERVO_CENTER = 90;  // Neutral position (servo midpoint)
const float MAX_TILT = 45.0;  // Max tilt angle before full correction
void setup() {
  Serial.begin(115200);
  while (!Serial);
  if (!IMU.begin()) {
    Serial.println("Failed to initialize BMI270/BMM150 IMU!");
    while (1);
  }
  stabilizerServo.attach(SERVO_PIN);
  stabilizerServo.write(SERVO_CENTER);  // start centered
  Serial.println("BMI270/BMM150 IMU initialized!");
  Serial.println("Pitch (°), Roll (°), Servo angle");
}
void loop() {
  float ax, ay, az;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    // Compute pitch and roll (radians)
    float pitch = atan2(-ax, sqrt(ay * ay + az * az));
    float roll  = atan2(ay, az);
    // Convert to degrees
    pitch *= 180.0 / PI;
    roll  *= 180.0 / PI;
    // Map roll angle to servo angle
    // Clamp roll between -MAX_TILT and +MAX_TILT
    roll = constrain(roll, -MAX_TILT, MAX_TILT);
    int servoAngle = map(roll, -MAX_TILT, MAX_TILT, 180, 0);
    // reverse mapping for corrective direction
    stabilizerServo.write(servoAngle);
    // Debug print
    Serial.print("Pitch: ");
    Serial.print(pitch, 1);
    Serial.print("°, Roll: ");
    Serial.print(roll, 1);
    Serial.print("°, Servo: ");
    Serial.println(servoAngle);
  }
  delay(100);
}