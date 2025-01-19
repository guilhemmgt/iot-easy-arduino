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

#define SPEAKER_PIN 15 // PORT D8
#define DOOR_SENSOR_PIN 3 // PORT RX 

// Active le débug part le port série, désactive le capteur de porte car le port RX ne peut pas être en entrée et en série
#define PRINT false 

// Taille du mot de passe 
const int BUFF_SIZE = 4;

#define BLANK '@'
#define VALIDATE 'A'
#define CANCEL '#'

// Delai en ms entre chaque requête au serveur 
#define POST_DELAY 5000


const byte Ligne = 4;
const byte Colonne = 4;

// Charactères du digicode
char hexaBouton[Ligne][Colonne] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

// Déclaration des ports du digicode (de gauche à droite)
byte Ligne_Pins[Ligne] = { 16, 5, 4, 0 };        // On connecte la ligne du digicode [Ports D0->D7]
byte Colonne_Pins[Colonne] = { 2, 14, 12, 13 };  // On connecte la colonne du digicode [Ports D4->D7]

// Instanciation du Keypad
Keypad mon_keypad = Keypad(makeKeymap(hexaBouton), Ligne_Pins, Colonne_Pins, Ligne, Colonne);

// Etat de la porte HIGH => ouvert | LOW => fermé
int doorState;

// Etat de l'alarme true => BIIIIIIP | false => silence
bool alarm_up;
// Indique si l'alarme est armée, i.e si elle surveille l'état de la porte
bool armed;

// Charactères lu sur le digicode
char buffer[BUFF_SIZE] = { BLANK, BLANK, BLANK, BLANK };

// Mot de passe du système
const char mdp[BUFF_SIZE] = { '1', '1', '1', '1' };

// Prochain instant pour sonner l'alarme
unsigned long nextNote;
// Prochain instant pour appeler le serveur
unsigned long nextCheck;

// SSID du réseau WiFi
const char* ssid = "mdp12345"; 
// Mot de passe du réseau WiFi
const char* password = "watchjojo";

// Adresse du serveur 
const char* host = "easy.kazzad.fr";
// URI où addresser les requêtes 
const char* url = "https://easy.kazzad.fr";

// ID de l'arduino, permet de différencier plusieurs systèmes EASY qui tournent en simultané
const String arduinoID = "TazerLesHamsters";

// Messages de communication avec le serveur
const String POST_DISTANT_CANCEL = "check_if_turn_off"; // Demande au serveur si l'alarme doit être éteinte
const String POST_DISTANT_ALARM_OFF = "alarm_is_off"; // Indique au serveur que l'alarme a été désamorcée
const String POST_DISTANT_ALARM_ON = "alarm_is_on"; // Indique au serveur que l'alarme a été déclenchée

// NTP Client pour avoir l'heure globale
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");


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

  if (PRINT) {
    Serial.begin(115200);
  } else {
    pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  }

  pinMode(SPEAKER_PIN, OUTPUT);

  init_WiFi();

  alarm_up = false;
  armed = true;
  nextNote = millis();
  nextCheck = millis();
}

void loop() {
  if (!alarm_up) {
    // Eteint l'alarme 
    noTone(SPEAKER_PIN);

    if(!PRINT) {
      // Lis l'état du capteur de fermeture de porte 
      doorState = digitalRead(DOOR_SENSOR_PIN);
    } else {
      // En mode débug attend 1 seconde puis redéclenche l'alarme
      delay(1000);
      doorState = true;
      armed = true;
    }
    alarm_up = doorState && armed;
    if (!armed) {
      // Si la porte est fermée réarme la sécurité
      armed = !doorState;
    }

    if(alarm_up) {
      // Déclenchement de l'alarme
      println("Alarme déclenchée !");
      https_post_server(POST_DISTANT_ALARM_ON);
    } else {
      println("Porte fermée.");
    }
  }

  if (alarm_up) {
    // Gestion de l'alarme
    if (millis() > nextNote) {
      // Prolonger le son de l'alarme
      tone(SPEAKER_PIN, NOTE_F6, 10000);
      nextNote = millis() + 5000;
    }

    if (mon_keypad.getKeys()) {
      // Si l'état d'une touche à changée on la lis
      read_digicode_and_check_password();
    }else if (millis() > nextCheck) {
      // Vérifier les désactivations à distance
      alarm_up = alarm_up and !https_post_server(POST_DISTANT_CANCEL);
      armed = alarm_up;
      nextCheck = millis() + POST_DELAY;
    }

    if (!alarm_up) {
      // Indiquer au serveur l'arrêt de l'alarme
      https_post_server(POST_DISTANT_ALARM_OFF);
    }

  }
}

// Envoyer une requête au serveur inspiré en partie de la solution suivante : https://www.makerhacks.com/ssl-post-esp8266/
bool https_post_server(String msg) {
  // Réponse du serveur
  bool answer = false;
  // Code de la réponse
  short response_code = 0;

  // Client WiFi pour connection https 
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient http;
  http.setReuse(true);
  http.setTimeout(1000);
  http.begin(*client, url);

  // Tampon temporel de la requête
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

// Lecture du digicode 
void read_digicode_and_check_password() {
  
  // Lis la touche enfoncée
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

      // Comparer les caratères lu avec le mot de passe 
      bool validate = true;
      for (int i = 0; i < BUFF_SIZE; i++) {
        validate = validate && buffer[i] == mdp[i];
      }

      if (validate) {
        println("\nmdp ok!\n");
        alarm_up = false;
        // Désenclenche l'alarme
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
  // Par défaut renvoie le caractère vide
  char res = '@';
  int i = 0;
  while (i < LIST_MAX)  // Scan the whole key list.
  {
    if (mon_keypad.key[i].stateChanged && mon_keypad.key[i].kstate == PRESSED)  // Only find keys that have changed state.
    {
      res = mon_keypad.key[i].kchar;
    }
    i++;
  }
  return res;
}

// Vide le buffer
void vider(char buff[]) {
  for (int i = 0; i < BUFF_SIZE; i++) {
    buff[i] = BLANK;
  }
}

// Fonction de débug pour écrire dans la comunication série 

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