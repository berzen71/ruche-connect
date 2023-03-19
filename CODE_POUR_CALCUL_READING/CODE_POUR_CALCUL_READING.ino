/* Code pour caLibration capteur CZL601 et ampli HX711
 *  
 * Peser le plateau de la balance avant le montage final
 *     et noter la valeur. 
 * Remonter le plateau sur la balance. 
 * Connecter la carte nano a l ordinateur.
 * Transferer le code dans l ide arduino.
 * Ouvrir le moniteur serie. 
 * Lire et noter la valeur du reading.Nous l appelerons reading 1.
 * Placer une tare de 2000 grammes sur la balance.
 * Noter a nouveau le reading.Nous l appellerons reading 2.
 * 
 * Calcul du coeficient    2000/readind 2-reading 1 =valeur coeficient 
 *    
 *     
 *Noter le coefficient ( ici 0.0222766763 pour les essais )
 *    il sera a rentrer dans le code de la balance 
 *    a la ligne     float reading = scale.read() * 0.0222766763;
 */

//******CODE   LECTURE DU READING**************//*

#include "HX711.h"
 
#define DOUT 3
#define CLK  2
 
HX711 scale;
 
void setup() {
  Serial.begin(57600);
  scale.begin(DOUT,CLK);
}
 
void loop() {
  if (scale.is_ready()) {
    long reading = scale.read();
    Serial.print("HX711 reading: ");
    Serial.println(reading);
  } else {
    Serial.println("HX711 not found.");
  }
delay(1000);
}
