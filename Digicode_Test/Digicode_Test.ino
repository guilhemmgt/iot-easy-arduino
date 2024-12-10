#include <Keypad.h>
const byte Ligne = 4;
const byte Colonne = 4;

char hexaBouton[Ligne][Colonne] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte Ligne_Pins[Ligne] = {1,3,15,13}; // On connecte la ligne du digicode 
byte Colonne_Pins[Colonne] = {12,14,2,0}; // On connecte la colonne du digicode

Keypad mon_keypad = Keypad( makeKeymap(hexaBouton), Ligne_Pins, Colonne_Pins, Ligne, Colonne);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  char keypad_matrix = mon_keypad.getKey();
  
  if (keypad_matrix){
    Serial.println(keypad_matrix);
  }
}
