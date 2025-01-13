#include <ESP8266WiFi.h>

const char* ssid = "TotallySecureNet"; // key in your own SSID
const char* password = "watchjojo"; // key in your own WiFi access point password
const char* host = "https://easy.kazzad.fr";

void setup() {
  Serial.begin(115200);
  delay(5000);
}

int value = 0;

void loop() {
  int numberOfNetworks = WiFi.scanNetworks();
  delay(3000);
  Serial.print("Found ");
  Serial.print(numberOfNetworks);
  Serial.println(" Networks");
  for (int i = 0; i < numberOfNetworks; i++)
  {
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID(i));
    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));
    Serial.println("-----------------------");
  }
  Serial.println("\n\n\n\n\n\n");
}