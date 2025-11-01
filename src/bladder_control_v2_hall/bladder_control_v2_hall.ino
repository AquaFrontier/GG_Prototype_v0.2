#include <SoftwareSerial.h>

// ===== USER SETTINGS =====
const float VIN_MAX   = 16.0;   // battery full
const float VOUT_MAX  = 12.0;   // cap effective motor volts
const int   POT_PIN   = A0;
const int   RAW_LEFT  = 0, RAW_RIGHT = 1023;
const int   RAW_CENTER= 772;    // your real STOP point
const int   DEAD_BAND = 10;     // counts around center = STOP
const bool  INVERT    = false;  // flip knob sense if needed

// Halls: analog, higher value near magnet; 1023 ≈ “at stop”
const int HALL_PIN_WATER_OUT = A1;   // end stop for FWD (water OUT)
const int HALL_PIN_WATER_IN  = A2;   // end stop for REV (water IN)
const int HALL_THRESH        = 980;  // trip near the top
const int HALL_HYST          = 15;   // release hysteresis

// ===== SABERTOOTH =====
SoftwareSerial saber(11,10);         // D10 TX -> S1 (RX unused)
const int MAX_CMD = (int)(127.0 * (VOUT_MAX / VIN_MAX) + 0.5);  // 12/16 => 95

static inline void sendM1(int s){     // s in [-MAX_CMD..+MAX_CMD]
  uint8_t b = 128;                    // 128 = STOP
  if (s > 0)      b = (uint8_t)constrain(s, 1, MAX_CMD);             // 1..95
  else if (s < 0) b = (uint8_t)(128 + constrain(-s, 1, MAX_CMD));    // 129..223
  saber.write(b);
}

int readAvg(int pin, int n=6){
  long acc=0; for(int i=0;i<n;i++){ acc += analogRead(pin); delayMicroseconds(200); }
  return (int)(acc/n);
}

void setup(){
  Serial.begin(115200);
  saber.begin(9600);                 // DIP: simplified serial @9600
  pinMode(POT_PIN, INPUT);
  pinMode(HALL_PIN_WATER_OUT, INPUT);
  pinMode(HALL_PIN_WATER_IN,  INPUT);
  Serial.print("MAX_CMD="); Serial.println(MAX_CMD);
}

// latch states to avoid chatter
bool outBlocked=false, inBlocked=false;

void loop(){
  // ---- Read controls ----
  int raw = analogRead(POT_PIN);                 // 0..1023
  int hallOut = readAvg(HALL_PIN_WATER_OUT);     // analog, higher near magnet
  int hallIn  = readAvg(HALL_PIN_WATER_IN);

  // ---- Map pot around real center to [-1..+1] ----
  int d = raw - RAW_CENTER;
  float x = 0.0f;
  if      (d > 0) x = (float)d / (float)(RAW_RIGHT - RAW_CENTER); // 0..+1 (FWD/OUT)
  else if (d < 0) x = -(float)(-d)/(float)(RAW_CENTER - RAW_LEFT);// 0..-1 (REV/IN)
  if (abs(d) <= DEAD_BAND) x = 0.0f;
  if (INVERT) x = -x;
  x = constrain(x, -1.0f, 1.0f);

  // ---- End-stop logic (assume peak=1023 is “at stop”) ----
  // Trip when >= HALL_THRESH, release when <= HALL_THRESH - HALL_HYST
  if (!outBlocked && hallOut >= HALL_THRESH) outBlocked = true;
  if ( outBlocked && hallOut <= (HALL_THRESH - HALL_HYST)) outBlocked = false;

  if (!inBlocked  && hallIn  >= HALL_THRESH) inBlocked  = true;
  if ( inBlocked  && hallIn  <= (HALL_THRESH - HALL_HYST)) inBlocked  = false;

  // If commanding into a blocked end, force stop. Away from it is allowed.
  if (x > 0 && outBlocked) x = 0.0f;   // FWD (water OUT) blocked
  if (x < 0 && inBlocked ) x = 0.0f;   // REV (water IN)  blocked

  // ---- Send command (capped to ±12 V effective) ----
  int cmd = (int)roundf(x * MAX_CMD);
  sendM1(cmd);

  // ---- Debug ----
  static uint32_t t=0;
  if (millis()-t > 150){
    t = millis();
    Serial.print("raw="); Serial.print(raw);
    Serial.print("  x="); Serial.print(x,3);
    Serial.print("  cmd="); Serial.print(cmd);
    Serial.print("  hallOut="); Serial.print(hallOut);
    Serial.print(outBlocked ? "(BLOCK)" : "");
    Serial.print("  hallIn="); Serial.print(hallIn);
    Serial.print(inBlocked ? "(BLOCK)" : "");
    Serial.println();
  }
}