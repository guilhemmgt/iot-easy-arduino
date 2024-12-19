#define PORT 1

void setup() {
  // put your setup code here, to run once:
  pinMode(PORT,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(PORT,HIGH);
  delay(900);
  digitalWrite(PORT,LOW);
  delay(500);

}
