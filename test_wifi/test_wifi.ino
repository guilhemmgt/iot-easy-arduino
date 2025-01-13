#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
// NTPClient by Fabrice Weinberg
#include <NTPClient.h>
#include <WiFiUdp.h>
const char* ssid = "mdp12345"; // key in your own SSID
const char* password = "watchjojo"; // key in your own WiFi access point password
const char* host = "easy.kazzad.fr";


const char* fingerprint = "E1 AB 24 CA 4F B0 33 7A 12 E8 72 91 96 E3 80 5B 14 AB 3A 6F";
// Get epoch time (source : https://randomnerdtutorials.com/epoch-unix-time-esp8266-nodemcu-arduino/)

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Function that gets current epoch time 
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  // Serial.print("Epoch : ");
  // Serial.println(now);
  return now;
}

void setup() {
  Serial.begin(115200);
  
  Serial.println();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  // We start by connecting to a WiFi network
  while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
  }
  // Serial.println();
  // Serial.println();
  // Serial.print("Connecting to ");
  // Serial.println(ssid);
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.println("Connexion en cours...");
  //   delay(1000);
  // }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();
  delay(2000);
}

// We now create a URI for the request
const char* url = "https://easy.kazzad.fr";

void loop() {
  delay(10000);
  short response_code = 0;
  Serial.print("connecting to ");
  Serial.println(host);
  // Use WiFiClient class to create TCP connections
  String payload = "";
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient http;
  http.setReuse(true);
  http.setTimeout(6000);

  // uint16_t httpsPort = 8443;
  // if (!client.connect(host, httpsPort)) {
  //   Serial.println("connection failed");
  //   return;
  // }
  http.begin(*client, url);

  Serial.print("Requesting URL: ");
  Serial.println(url);
  // This will send the request to the server


  char* timestamp[10];
  unsigned long epoch = getTime();
  String request = String();
  request.concat("{\"key\":\"TazerLesHamsters\",\"message\":\"check_if_turn_off\",\"timestamp\":\"");
  request.concat(epoch);
  request.concat("\"}");
  Serial.println(request);

  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(request);
  if (httpResponseCode > 0) {
    http.writeToStream(&Serial);

    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] ... code: %d\n", httpResponseCode);

    // file found at server
    if (httpResponseCode >= 200 and httpResponseCode <= 299) {
      response_code = 1;
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] ... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  // Serial.println();
  // Serial.println("closing connection");
  // Free resources
  http.end();
}



