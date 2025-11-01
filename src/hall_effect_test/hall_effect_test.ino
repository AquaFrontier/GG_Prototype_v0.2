const int HALL_PIN = A0;   // Analog input from Hall sensor

void setup() {
  Serial.begin(115200);
  Serial.println("Linear Hall Effect Sensor Test");
}

void loop() {
  int rawValue = analogRead(HALL_PIN);           // 0–1023 range (10-bit ADC)
  float voltage = rawValue * (5.0 / 1023.0);     // Convert to voltage
  Serial.print("Raw: ");
  Serial.print(rawValue);
  Serial.print("\tVoltage: ");
  Serial.print(voltage, 3);
  Serial.println(" V");
  delay(100);
}