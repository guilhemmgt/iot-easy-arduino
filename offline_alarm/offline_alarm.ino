#define SPEAKER_PIN 15 
#define DOOR_SENSOR_PIN 3 
#define PRINT false
const int BUFF_SIZE = 4;

#define BLANK '@'
#define VALIDATE 'A'
#define CANCEL '#'

#include "pitches.h"
#include <Key.h>
#include <Keypad.h>


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
// notes in the melody:
int melody[] = {

  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};
int doorState;
int thisNote;
/*int */
bool alarm_up;
char buffer[BUFF_SIZE] = { BLANK, BLANK, BLANK, BLANK };
const char mdp[BUFF_SIZE] = { '1', '1', '1', '1' };
unsigned long nextNote = 0;
bool armed;
void setup() {
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);

  if (PRINT) {
    Serial.begin(9600);
  } else {
    pinMode(SPEAKER_PIN, OUTPUT);

  }
  println("Début");
  // iterate over the notes of the melody:
  thisNote = 0;
  alarm_up = false;
  armed = true;
  nextNote = 0;
}

void loop() {
  if (!alarm_up) {
    doorState = digitalRead(DOOR_SENSOR_PIN);
    alarm_up = doorState && armed;
    if (!armed) {
      armed = !doorState;
    }
    println("ALLO ?!");
  }
  if (alarm_up) {
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
    // Gestion de l'alarme
    if (millis() > nextNote) {
      play_next_note();
    }
  }
  // no need to repeat the melody.
}
void play_next_note() {
  if(!PRINT) {
    noTone(SPEAKER_PIN);
    // to calculate the note duration, take one second divided by the note type.

    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.

    int noteDuration = 1000 / noteDurations[thisNote];

    tone(SPEAKER_PIN, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.

    // the note's duration + 30% seems to work well:

    int pauseBetweenNotes = noteDuration * 1.30;

    nextNote = millis() + pauseBetweenNotes;

    // stop the tone playing:

    thisNote = (thisNote + 1) % (sizeof(melody) / sizeof(int));
  } else {
    // println("BIP");
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