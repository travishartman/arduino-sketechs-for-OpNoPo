const int splSensor = A0;   // the SPL output is connected to analog pin 0

void setup() {
 Serial.begin(38400);
}

void loop() {
  Serial.println(analogRead(splSensor), DEC);
  delay(200);  // delay to avoid overloading the serial port buffer
}
