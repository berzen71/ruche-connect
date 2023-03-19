/*Code pour balance avec capteur CZL 601 100KG et
amplificateur type HX711.

/*Pour les réglages il faut d abord utiliser le code de calibration
* du capteur CZL 601 et calculer le coeficient a rentrer
* ligne( long reading = scale.read() * 0.02183406;)
* ici 0.02183406 pour les essais.Cette valeur est propre a chaque gauge de contrainte 
* il faut refaire la calibration pour votre gauge et noter vos valeurs 
* (voir code de calibration)
*
* Peser le plateau de la balance avec une balance du commerce et rentrer
* la valeur de celui ci dans la ligne du code:(int poidsPlateau=1888;)
* ici 1888 pour les essais.
*
* Ensuite il faut remonter le plateau et ouvrir le moniteur série
* et noter la valeur de poids affichée.
* Cette valeur est a rentrer ligne du code :(int correctionTare=185;)
* ici 185 pour les essais.
*
* Recompiler et transferer dans la carte nano et reouvrir le moniteur série
* La valeur doit etre de zero ou proche de celui ci.
*
* La balance est prete à l emploi.
* La ruche peut etre installée sur la balance.
*
* En cas de coupure de l alimentation le programme reprend ses valeurs initiales .
* Il n y a pas besoin de refaire la manipulation.
*
* La balance peut etre deplacer sous une autre ruche.
*/

//*******declarations des bibliotèques*********//
#include "HX711.h"
#include <Wire.h>//bus I2C
#include <LiquidCrystal_I2C.h>//ecran lcd /non utilisé 

LiquidCrystal_I2C lcd(0x27,20,4); //adresse de l ecran sur le bus I2C #define DOUT  3

//********declaration du type de l amplificateur *******//
HX711 scale;

//********pin  de l ampli HX711*********//
#define DOUT 3
#define CLK  2

union my_float{
  float val;
  unsigned char b[4];
} f;

//********déclaration des variables**********//
int correctionTare=185;//valeur a modifier pour le tarage apres calibration
int poidsPlateau=1888; //pesée le plateau de la balance et modifier  la valeur poids plateau 
                       //ici 1888 g pour la balance d essai 

float grams =0;
float p2 = 0;

void setup() {
  Serial.begin(9600);

  Wire.begin(3);//ouverture du bus I2C adresse 3
  Wire.onRequest(requestEvent);//transmission

  scale.begin(DOUT, CLK);

  }//fin du setup
 
void loop() {

  if (scale.is_ready()) {
    float reading = scale.read() * 0.0222766763;//valeur a rentrer après la calibration 
    float grams = (float)reading ;
    grams = grams - (poidsPlateau+correctionTare);
    p2 = grams / 1000 ;//transforme en kilo
    
//******affichage dans le moniteur serie******//
    Serial.print("HX711 reading: ");   
    Serial.print(p2);
    Serial.println(" KG");

  }
  else 
  {
   Serial.println("HX711 not found.");
  }
   delay (100);
  }

void requestEvent()//fonction pour la transmission du poids
 {
  f.val = p2;
  Wire.write(f.b, 4);
 }
