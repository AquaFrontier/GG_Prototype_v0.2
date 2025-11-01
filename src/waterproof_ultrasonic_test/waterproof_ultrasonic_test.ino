unsigned char buffer_RTT[4] = {0};
uint8_t CS;
#define MESSAGE 0x55
int Distance = 0;
void setup() {
  Serial.begin(9600);     // USB serial (for monitor)
  Serial1.begin(115200);    // Hardware UART (D0=TX, D1=RX)
  delay(2000);
  Serial.println("Starting distance read...");
}
void loop() {
  Serial1.write(MESSAGE);       // send command byte
   //Serial.println("pinging");
  //delay(100);
  if (Serial1.available() > 0) {
    Serial.println("receiving");
     delay(4);               // small wait for full frame
    if (Serial1.read() == 0xFF) {
      buffer_RTT[0] = 0xFF;
      for (int i = 1; i < 4; i++) {
        buffer_RTT[i] = Serial1.read();
      }
      CS = buffer_RTT[0] + buffer_RTT[1] + buffer_RTT[2];
      Serial.println(buffer_RTT[3]);
      if (buffer_RTT[3] == CS) {
        Distance = (buffer_RTT[1] << 8) + buffer_RTT[2];
        Serial.print("Distance: ");
        Serial.print(Distance);
        Serial.println(" mm");
      }
    }
  }
}

