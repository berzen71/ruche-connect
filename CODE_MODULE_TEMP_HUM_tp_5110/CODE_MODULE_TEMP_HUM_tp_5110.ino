#include "DHT.h"
#include <SPI.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"
#define VREF  3.3//batterie
#define PIN_POTENTIOMETRE A0  //batterie 
#define RF24_CEPIN 9 // broche ce du spi
#define RF24_CSPIN 10 // broche cs du spi

# define PIN_STOP 4 //led visu selection 24h

float voltage =0;
float tensionbat=0;
float pourcentage=0;
float ref =0;

float BatterieMaxi=100;
float BatterieMini=73;
float BatterieActuelle; 
float diff=0;

byte varCompteur = 0; // La variable compteur

RF24 radio(RF24_CEPIN, RF24_CSPIN); // le module
char payload[32+1] = {0}; // champ payload (de 0 à 32 octets)
const uint64_t txPipe = 0xE8E8E8E8E8LL; // adresse du canal de transmission
// cette adresse doit être identique à l'adresse du canal de réception du récepteur
const uint64_t rxPipe = 0xF6F6F6F6F1LL; // adresse du canal de réception
// cette adresse doit être identique à l'adresse du canal de transmission du récepteur
/***************************************************************************/


#define DHTPIN 2     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.


#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321


// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

byte Compteur  = 0; // La variable compteur (TIMER2 )
byte Compteur1 = 0; // La variable compteur pour interval )

// déclaration des variables
int humiditee = 0 ;
int temperature = 0 ;
int interval=2;


void setup(){

  
Serial.begin(9600);
Serial.println(F("DHTxx test!"));

//*****Compteur 2 pour base de tps fonction interval envoie  message  
    cli(); // Désactive l'interruption globale
    bitClear (TCCR2A, WGM20); // WGM20 = 0
    bitClear (TCCR2A, WGM21); // WGM21 = 0 
    TCCR2B = 0b00000110; // Clock / 256 soit 16 micro-s et WGM22 = 0
    TIMSK2 = 0b00000001; // Interruption locale autorisée par TOIE2
    sei(); // Active l'interruption globale

    
    dht.begin();
// communication
    printf_begin();
    radio.begin();
    radio.printDetails(); // affiche les caractéristiques spécifiques au module

// Initialisations radio :
// - validation d'une gestion dynamique de la taille du payload
radio.enableDynamicPayloads(); // payload de taille variable
// - réglage de la vitesse de transmission des données
radio.setDataRate(RF24_1MBPS); // 1 Mbits/s
// - réglage de la puissance de l'amplificateur de transmission
radio.setPALevel(RF24_PA_LOW); // puissance maxi
// - réglage de la fréquence du canal de transmission
radio.setChannel(0x10); // ici fréquence = 2,4GHz + 16*1Mhz
// - réglage du nombre et du retard des réitérations avant l'abandon des transmissions
// delais 4 ms, 15 essais ( 0 - 250us, 15 - 4000us )
radio.setRetries(15,15); // essais & délais
radio.openWritingPipe(txPipe); // ouverture du canal logique de transmission
radio.openReadingPipe(1,rxPipe); // ouverture du canal de réception (n° pipe de 0 à 5, adresse)
radio.setAutoAck(true); // valide les acquittements automatiques
radio.startListening(); // commence l'écoute sur les canaux ouverts pour la lecture
}
/***************************************************************************/
void loop() {
    voltage=analogRead(A0);
    tensionbat =map(voltage,0,1023,0,100);

   BatterieActuelle =tensionbat*2;
   pourcentage=((BatterieActuelle-BatterieMini)/(BatterieMaxi-BatterieMini))*100;
    
   
//*********appel de la fonction  pour stopper le nrf 24l01************//
      if (Compteur1>interval)//condition 10s de marche
        { 
         NRF24L01Stop(); //appel de la fonction
        }
// Wait a few seconds between measurements.
  delay(1000);
// à faire toutes les 500 ms : {
// lecture des 3 voies de l'ADXL335
 humiditee =dht.readHumidity()  ;
int temperature = dht.readTemperature() ;


  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));

    return;
  }

    // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
  Serial.print(F("pourcentage "));
  Serial.print(pourcentage);
  Serial.print(F("% ")); 
  Serial.print(F("voltage "));
  Serial.print(BatterieActuelle);
//Serial.print(F("v ")); 
  Serial.print('\n');


// formation du payload"%d"
//sprintf(payload,"T%dR%dV%d", (int)humiditee, (int)temperature, (int)DeltaVertical);
sprintf(payload,"T%dR%dV%d", (int)humiditee, (int)temperature, (int)pourcentage);
Serial.println(payload); //Serial.print('\t');
 if (!radio.testCarrier()) { // vérification si canal disponible
// envoi du payload
radio.stopListening(); // en émission
radio.write((void *)payload, strlen(payload));
radio.startListening();
 }
// }
}


//********** routine d'interruption du timer2*******************//
ISR (TIMER2_OVF_vect){
    // 256-6 --> 250X16uS = 4mS
    // Recharge le timer pour que la prochaine interruption se déclenche dans 4mS
    TCNT2 = 6;

    if (Compteur++ == 250) {
      //250*4mS = 1S 
      Compteur = 0;
      
     }
  if (Compteur == 250)
     {
      Compteur1 = Compteur1 + 1;     
     }
    }

 //********fonction pour séquence d arret du NRF24l01 sur la pin 4
void NRF24L01Stop(){
  if (Compteur1>=interval){
      pinMode(4, OUTPUT);
      digitalWrite(4, LOW);
      delay(500);
      digitalWrite(4, HIGH);
      delay(500);
      digitalWrite(4, LOW);
      delay(500);
      }
     }
