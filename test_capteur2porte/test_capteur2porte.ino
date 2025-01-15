const int DOOR_SENSOR_PIN = 13;
int doorState;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);

}

void loop() {
  // put your main code here, to run repeatedly:

  doorState = digitalRead(DOOR_SENSOR_PIN);

  if (doorState == HIGH) {
    Serial.println("JOUR");
  } else {
    Serial.println("NUIT");
  }
  delay(100);
}
