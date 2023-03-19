  # define DEBUGtoSERIAL 1


//******declaration bibliotheque*******//
#include <SPI.h> //comunication nrf24L01 + arduino balance 
#include <RF24_config.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

#include <Wire.h> //bibliotheque pour bus I2C communication balance 
#include <EEPROM.h>//bibliotheque pour ecrire et lire dans l EEPROM
//********declaration pin d entree du Potentiometre reglage interval de tps entre les message
#define PIN_POTENTIOMETRE A0 //analod pin A0 Potentiometre réglage interval time 

//********declaration pin d entrée de l anémomètre********//
#define PIN_ANEMOMETRE 4 //analog pin A4

//*********declaration des boches CE CS du module NRF24L01 pour Atmega UNO ou Nano******//
//#define RF24_CEPIN 9 // broche ce du spi
//#define RF24_CSPIN 10 // broche cs du spi

//*********declaration des boches CE CS du module NRF24L01 pour Atmega MEGA R3**********//
#define RF24_CEPIN 7 // broche CE du spi
#define RF24_CSPIN 8 // broche CS du spi

//**********déclaration des variables pour traiter le signal du potar analog pin A0*****//
int potar  =0;
int valpot =0;//recupere la valeur du potar

//***********definition des valeur de base pour l interval de tps entre les SMS*********//
//        base de tps du compteur 1s X 21600 = 6h
//        base de tps du compteur 1s X 43200 = 12h
//        base de tps du compteur 1s X 86400 = 24h
int unsigned long timeMsg1=100;//pour test 
int unsigned long timeMsg2=21600;
int unsigned long timeMsg3=43200;
int unsigned long timeMsg4=86400;

//*******variable pour chargement de la valeur interval de tps entre les SMS************//
unsigned long interval=0;//parametre utilisé dans la fonction intervalTime

//**********déclaration des variables recuperation des valeurs du DHT 22****************//
int b = 0;//humiditée recuperation de la variable humiditee transmise par nrf 24l01
int c = 0;//température recuperation de la variable temperature  transmise par nrf 24l01
int d = 0;//etat batterie

//*****************Variable pour recuperer la valeur poids*********//  
 
float p=0;
 
 

//**********déclaration des variablespour anemometre**************//
int windSpeed = 0;//vitesse du vent
int sensorValue=0; //pour entree anemometre
 //…/...
 
//**********declaration sortie led visu comptage compteur 2 / 1s*********//
const byte Led = 22; // Pour utiliser la LED du module
#define LedToggle digitalWrite (Led, !digitalRead(Led))//inversion de la led 

//*********declaration des sorties LED visu interval de tps entre les messages**********//
# define PIN_LED_GREEN   40 //led visu selection test au demarage 
# define PIN_LED_BLUE    41 //led visu selection 6h
# define PIN_LED_YELLOW  42 //led visu selection 12h
# define PIN_LED_RED     43 //led visu selection 24h

//********declaration de la pin de commande de demarrage du SIM900*************//
# define PIN_SIM_POWER   9 //pin de demarrage du SIM 900

 byte Compteur = 0; // La variable compteur (TIMER2 )
 unsigned long Compteur1 = 0;

/** La structure permettant de stocker les données */
struct MaStructure {
   int valeur_1;
   int valeur_2;
   int valeur_3;
}; // Ne pas oublier le point virgule !

 
