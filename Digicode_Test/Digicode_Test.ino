#include <Key.h>
#include <Keypad.h>


const byte Ligne = 4;
const byte Colonne = 4;

char hexaBouton[Ligne][Colonne] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte Ligne_Pins[Ligne] = {16,5,4,0}; // On connecte la ligne du digicode [Ports D0->D7]
byte Colonne_Pins[Colonne] = {2,14,12,13}; // On connecte la colonne du digicode [Ports D4->D7]

Keypad mon_keypad = Keypad( makeKeymap(hexaBouton), Ligne_Pins, Colonne_Pins, Ligne, Colonne);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Début");
}

void loop() {
  // put your main code here, to run repeatedly:
  // inspiré du code MultKey.ino des exemples de la bibliothèque Keypad.h
  if (mon_keypad.getKeys())
  {
    for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
        {
            if ( mon_keypad.key[i].stateChanged && mon_keypad.key[i].kstate == PRESSED )   // Only find keys that have changed state.
            {
              Serial.print(mon_keypad.key[i].kchar);
            }
        }
  }
}
