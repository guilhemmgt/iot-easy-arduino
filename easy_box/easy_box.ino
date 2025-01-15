#define SPEAKER_PIN 15
#define DOOR_SENSOR_PIN 3
#define PRINT false
const int BUFF_SIZE = 4;

#define BLANK '@'
#define VALIDATE 'A'
#define CANCEL '#'

#define POST_DELAY 1000
// #define POST_DISTANT_CANCEL "check_if_turn_off"
#include "pitches.h"
#include <Key.h>
#include <Keypad.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
// NTPClient by Fabrice Weinberg
#include <NTPClient.h>
#include <WiFiUdp.h>
const byte Ligne = 4;
const byte Colonne = 4;

char hexaBouton[Ligne][Colonne] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte Ligne_Pins[Ligne] = { 16, 5, 4, 0 };        // On connecte la ligne du digicode [Ports D0->D7]
byte Colonne_Pins[Colonne] = { 2, 14, 12, 13 };  // On connecte la colonne du digicode [Ports D4->D7]

Keypad mon_keypad = Keypad(makeKeymap(hexaBouton), Ligne_Pins, Colonne_Pins, Ligne, Colonne);

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};
int doorState;
int thisNote;

bool alarm_up;
char buffer[BUFF_SIZE] = { BLANK, BLANK, BLANK, BLANK };
const char mdp[BUFF_SIZE] = { '1', '1', '1', '1' };
unsigned long nextNote = 0;
bool armed;

const char* ssid = "mdp12345"; // key in your own SSID
const char* password = "watchjojo"; // key in your own WiFi access point password
const char* host = "easy.kazzad.fr";
// We now create a URI for the request
const char* url = "https://easy.kazzad.fr";
const String arduinoID = "TazerLesHamsters";
const String POST_DISTANT_CANCEL = "check_if_turn_off";
const String POST_DISTANT_ALARM_OFF = "alarm_is_off";
const String POST_DISTANT_ALARM_ON = "alarm_is_on";
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
unsigned long nextCheck;
// Function that gets current epoch time 
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

void init_WiFi() {
  // Connection au WiFi
  println("Début");
  print("Connecting to ");
  println(ssid);

  WiFi.begin(ssid, password);
  // We start by connecting to a WiFi network
  while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   print(".");
  }

  println("");
  println("WiFi connected");
  println("IP address: ");
  if (PRINT)  Serial.println(WiFi.localIP());
  timeClient.begin();
}

void setup() {
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);

  if (PRINT) {
    Serial.begin(115200);
  } 
  pinMode(SPEAKER_PIN, OUTPUT);
  init_WiFi();

  // iterate over the notes of the melody:
  thisNote = 0;
  alarm_up = false;
  armed = true;
  nextNote = 0;
  nextCheck = 0;
}

void loop() {
  if (!alarm_up) {
    noTone(SPEAKER_PIN);
    doorState = digitalRead(DOOR_SENSOR_PIN);
    alarm_up = doorState && armed;
    if (!armed) {
      armed = !doorState;
    }
    if(alarm_up) {
      https_post_server(POST_DISTANT_ALARM_ON);
    } else {
      println("ALLO ?!");
    }
  }
  if (alarm_up) {
    // Gestion de l'alarme
    if (millis() > nextNote) {
      tone(SPEAKER_PIN, NOTE_F6, 10000);
      nextNote = millis() + 5000;
    }
    read_digicode_and_check_password();

    if (millis() > nextCheck) {
      alarm_up = alarm_up and !https_post_server(POST_DISTANT_CANCEL);;
      armed = alarm_up;
      nextCheck = millis() + POST_DELAY;
    }

    if (!alarm_up) {
      https_post_server(POST_DISTANT_ALARM_OFF);
    }

  }
}
bool https_post_server(String msg) {
  bool answer = false;
  short response_code = 0;

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient http;
  http.setReuse(true);
  http.setTimeout(1000);

  http.begin(*client, url);

  char* timestamp[10];
  unsigned long epoch = getTime();
  String request = String();
  request.concat("{\"key\":\"");
  request.concat(arduinoID);
  request.concat("\",\"message\":\"");
  request.concat(msg);
  request.concat("\",\"timestamp\":\"");
  request.concat(epoch);
  request.concat("\"}");
  if(PRINT) Serial.println(request);

  http.addHeader("Content-Type", "application/json");
  
  // This will send the request to the server
  int httpResponseCode = http.POST(request);
  if (httpResponseCode > 0) {

    // file found at server
    if (httpResponseCode >= 200 and httpResponseCode <= 299) {
      response_code = 1;
      String payload = http.getString();

      DynamicJsonDocument doc(1024);

      // You can use a String as your JSON input.
      // WARNING: the string in the input  will be duplicated in the JsonDocument.
      deserializeJson(doc, payload);
      answer = doc.as<JsonObject>()["message"].as<bool>();
    }
  }
  // Free resources
  http.end();
  return answer;
}

void read_digicode_and_check_password() {
  // Gestion du digicode
    char input = get_current_key();
    if (input != BLANK) {
      print("Input read : ");
      println_var(&input);
      if (input == VALIDATE) {

        println("Validation");
        print("mdp =");
        println(mdp);
        print("buf =");
        println(buffer);

        // vérfier mdp
        bool validate = true;
        for (int i = 0; i < BUFF_SIZE; i++) {
          validate = validate && buffer[i] == mdp[i];
        }
        if (validate) {
          println("\nmdp ok!\n");
          alarm_up = false;
          armed = false;
          noTone(SPEAKER_PIN);
        }
        println("fin de validation");
      } else {
        print("Modification du buffer.");
        print(buffer);
        print(" => ");

        // remplacer le premier charactère vide du buffer
        int i = 0;
        while (i < BUFF_SIZE) {
          if (buffer[i] == BLANK) {
            buffer[i] = input;
            break;
          }
          i++;
        }
        if (i == BUFF_SIZE) {
          vider(buffer);
          buffer[0] = input;
        }
        println(buffer);
      }
      if (input == VALIDATE || input == CANCEL) {
        // vider le buffer
        print(buffer);
        print(" => ");
        vider(buffer);
        println(buffer);
      }
    }
}

// Récupère la touche préssée
char get_current_key() {
  char res = '@';
  if (mon_keypad.getKeys()) {
    int i = 0;
    while (i < LIST_MAX)  // Scan the whole key list.
    {
      if (mon_keypad.key[i].stateChanged && mon_keypad.key[i].kstate == PRESSED)  // Only find keys that have changed state.
      {
        print("key pressed =");
        res = mon_keypad.key[i].kchar;
        println(&res);
      }
      i++;
    }
  }
  return res;
}
// Vide le buffer
void vider(char buff[]) {
  for (int i = 0; i < BUFF_SIZE; i++) {
    buff[i] = BLANK;
  }
}

void print(const char* msg) {
  if (PRINT) {
    Serial.print(msg);
  }
}
void println(const char* msg) {
  if (PRINT) {
    Serial.println(msg);
  }
}

void print_var(char msg[]) {
  if (PRINT) {
    Serial.print(msg);
  }
}

void println_var(char msg[]) {
  if (PRINT) {
    Serial.println(msg);
  }
}