#include "pitches.h"
#define SPEAKER_PIN 15
#define DOOR_SENSOR_PIN 5
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
void setup() {
  pinMode(SPEAKER_PIN,OUTPUT);
  Serial.begin(9600);
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  // iterate over the notes of the melody:
  thisNote = 0;
}

void loop() {
  doorState = digitalRead(DOOR_SENSOR_PIN);
  if (doorState == HIGH) {

    // to calculate the note duration, take one second divided by the note type.

    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.

    int noteDuration = 1000 / noteDurations[thisNote];

    tone(SPEAKER_PIN, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.

    // the note's duration + 30% seems to work well:

    int pauseBetweenNotes = noteDuration * 1.30;

    delay(pauseBetweenNotes);

    // stop the tone playing:

    noTone(8);
    thisNote = (thisNote+1)%sizeof(melody);
  }
  // no need to repeat the melody.
}
