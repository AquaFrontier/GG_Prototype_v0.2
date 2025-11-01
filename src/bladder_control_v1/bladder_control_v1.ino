#include <SoftwareSerial.h>

const int POT_PIN = A0;
SoftwareSerial saber(11,10);   // D10 TX -> S1

void setup(){
  Serial.begin(115200);
  saber.begin(9600);           // DIP set for Simplified Serial @ 9600
}

void loop(){
  int raw = analogRead(POT_PIN);      // 0..1023
  int centered = raw - 512;           // -512..+511

  // map to -127..+127
  int speed = map(centered, -512, 511, -127, 127);

  uint8_t cmd;
  if (speed > 0)      cmd = constrain(speed,1,127);          // forward
  else if (speed < 0) cmd = 128 + constrain(-speed,1,127);   // reverse
  else                cmd = 128;                             // stop

  saber.write(cmd);

  Serial.print("raw="); Serial.print(raw);
  Serial.print("  speed="); Serial.print(speed);
  Serial.print("  cmd="); Serial.println(cmd);

  delay(50);
}