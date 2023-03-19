//***Programme pour une ruche connectée en vue de recuperer les valeurS de T° d humiditée de poids de la ruche 
//   ajout d'un anémometre pour capter la vitesse du vents. 
//   Principe :Température et humiditée
//               récupération des valeurs de t° et d humiditée par 
//               capteur type DHT 22  et transmissions des valeurs 
//               à la carte principale par module nRF24l01
//             :Pesage
//               Systeme de pesée independant par jauge de contrainte 
//               type CZL 601 AC 100KG.Transmission des valeurs de pesée
//               par bus I2C a la carte principale
//              :Anémomètre
//                CALT 9-30 V DC Capteur de vitesse du vent 0-45 m/s
//                Sortie analogique 0-5v
//              :Transmission a l utilisateur
//                Par module GMS SIM 900 
//                option 2 numeros de tel
//***Essai realisé avec une carte free abonnement 2 euro ( carte desimlockée)

//*****Materiels utilisés**********************//
//              T° et humiditée_ _ _ _ _ _ _ _:DHT22
//              transmission T°et humiditée_ _:EBYTE Module sans fil 2,4 GHz
//                                               nRF24L01+PA+LNA RF module 
//                                               longue portée 2500 m 100 mW SPI 
//              Pesage_ _ _ _ _ _ _ _ _ _ _ _ _:jauge de contrainte 
//                                               CZL 601 AC 100KG                     
//                                             :HX711 capteurs de pesée à double canal
//                                               24 bits Précision
//                                             :HD44780 1602 LCD Module Affichage 
//                                             :I2C Serial Adapter Board Module Interface
//                                               pour LCD Display 1602 et 2004
//              Transmission SMS_ _ _ _ _ _ _ _:GSM SIM 900 S2-1040S-Z1K18
//                                               carte SIM M2M /machine to machine
//              Cartes atmega_ _ _ _ _ _ _ _ _ : MEGA R3 x1
//                                               NANO    x2  
//              divers_ _ _ _ _ _ _ _ _ _ _ _ _:carte mère support composants
//                                              fabrication Easy eda


//   rentrer le numero de portable  dans cette ligne  a la place des zero
//                 String numero[1]= {"00000000\""};//pour 1 numeros
//                 String numero[2]= {"00000000\"","00000000\""};//pour 2 numeros
//                 commenter et decommenter les lignes selon le choix 
//                 pour deux numero voir  commentaires dans le code 
   
//***************CODE PRINCIPAL RUCHE CONNECTEE**************//   
   
   
   #include "config.h"//appel des parametre de l onglet config.h

//**********code pour SIM 900***********//
    String message = "Eagle Robotics is Awesome";
    // String numero[2]= {"00000000\"","00000000\""}; // Numéros des destinataires. Si vous ajoutez plus de numéros,
                                                      // pensez à modifier aussi le nombre entre les crochets de numero 
                                                      // et aussi la valeur à laquelle "a" doit être inférieure dans la boucle "For" plus bas.
                                                      // ligne 305 306 fonction "void transmission"
    String numero[1]= {"00000000\""};//pour 1 numero
    #define DEBUGtoSERIAL 2

    RF24 radio(RF24_CEPIN, RF24_CSPIN); // le module

    char payload[32+1] = {0}; // champ payload (de 0 à 32 octets)
    const uint64_t rxPipe = 0xE8E8E8E8E8LL; // adresse du pipe de réception
    const uint64_t txPipe = 0xF6F6F6F6F1LL; // adresse du pipe de transmission
    // cette adresse doit être identique à l'adresse du canal de réception de l'émetteur

union my_float{
  float val;
  unsigned char b[4];
} f;

void setup(){
    Serial.begin(9600);//affichage dans le moniteur serie
    Serial2.begin(9600);//demarrage comunication avec le SIM 900
    Wire.begin(); // Rejoindre le bus à  I2C  Pas besoin d adresse pour le maitre
   
//******declaration des sortie LED visu selection interval entre les SMS
    pinMode(PIN_LED_GREEN  , OUTPUT);//LED_GREEN   visu selection interval 4 h
    pinMode(PIN_LED_BLUE   , OUTPUT);//LED_BLUE   visu selection interval 4 h
    pinMode(PIN_LED_YELLOW , OUTPUT);//LED_YELLOW visu selection interval 12 h
    pinMode(PIN_LED_RED    , OUTPUT);//LED_REDled visu selection interval 24 h    

//declaration de la pin 9 en sortie demmarage du SIM 900**********//
    pinMode(PIN_SIM_POWER  , OUTPUT);
    
    pinMode (Led, OUTPUT);//pin declaré en sortie /visu comptage
   
//*****Compteur 2 pour base de tps fonction interval envoie  message  
    cli(); // Désactive l'interruption globale
    bitClear (TCCR2A, WGM20); // WGM20 = 0
    bitClear (TCCR2A, WGM21); // WGM21 = 0 
    TCCR2B = 0b00000110; // Clock / 256 soit 16 micro-s et WGM22 = 0
    TIMSK2 = 0b00000001; // Interruption locale autorisée par TOIE2
    sei(); // Active l'interruption globale

//******* communication NRF 24L01******//
    printf_begin();
    radio.begin();
    radio.printDetails(); // affiche les caractéristiques spécifiques au module
    // Initialisations radio :
    // - validation d'une gestion dynamique de la taille du payload
    radio.enableDynamicPayloads(); // payload de taille variable
    // - réglage de la vitesse de transmission des données
    radio.setDataRate(RF24_1MBPS); // 1 Mbits/s
    // - réglage de la puissance de l'amplificateur de transmission
    radio.setPALevel(RF24_PA_MAX); // puissance maxi
    // - réglage de la fréquence du canal de transmission (même fréquence qu'en émission)
    radio.setChannel(0x10); // ici fréquence = 2,4GHz + 16*1Mhz
    // - réglage du nombre et du retard des réitérations avant l'abandon des transmissions
    // delais 4 ms, 15 essais ( 0 - 250us, 15 - 4000us )
    radio.setRetries(4,15); // essais & délais
    radio.openWritingPipe(txPipe); // ouverture du canal logique de transmission
    radio.openReadingPipe(1,rxPipe); // ouverture du canal de réception (n° pipe de 0 à 5, adresse) 
  
    
    radio.setAutoAck(true); // valide les acquittements automatiques 
    
    radio.startListening(); // commence l'écoute sur les canaux ouverts pour la lecture  
  }

void loop(){
    Wire.requestFrom(3, 4);    // demande de l envoie par le maitre  par l'esclave adresse 3
                               //de la valeur poids 
   int i = 0;
   while (Wire.available()) 
    { // peripheral may send less than requested
        uint8_t b = Wire.read(); // receive a byte as character
        f.b[i] = b;
        i++;
    }            
    p = f.val;//Wire.read();    // Reception de l'octet (byte) comme caractère
  
//**********traitement de l entree anémomètre 
    int sensorValue = analogRead(A4);
    windSpeed = map (sensorValue,0,1023 ,0,45);//vitesse du vent 0...30ms
    
//**********Potar reglage tps interval    
    potar=analogRead(A0);
    valpot=map(potar ,0,1023,0 ,800);


    MaStructure ms;   
      ms.valeur_1 = b;// recuperation % humiditee
      ms.valeur_2 = c;// recuperation temperature
      ms.valeur_3 = d;// recuperation etat batterie

//*****Reception données du NRF 24L01************//

 
    if ((radio.available())&&Compteur1>=5) { // verif si une transmission est arrivé ?
//if (Compteur1>=5) { // verif si une transmission est arrivé ? 
     int pls = radio.getDynamicPayloadSize(); // récupération de la taille du payload
     if (pls >= 1 && pls < 32) { // forcément
     radio.read((void*)payload, pls); // lecture du payload
     payload[pls] = 0;
      sscanf(payload, "T%dR%dV%d",&b,&c,&d); // lecture de la chaîne formatée à l'émission

     
     EEPROM.put(0, ms);//ecriture des variables dans l EEPROM
     
     Serial.print(F("Humidity: "));
     Serial.print(b);
     //Serial.print(" "); // affichage des 3 octets émis
     Serial.print(F("%  Temperature: "));
     Serial.print(c);
     Serial.print(F("°C "));
     Serial.print(F("Poids: "));
     Serial.print(p); 
     Serial.print(F("kg "));
     Serial.print(F("WindSpeed: "));
     Serial.print(windSpeed); 
     Serial.print(F("M/S "));
      Serial.print(F("cpt "));
     Serial.print(Compteur1);
     Serial.print(F("cpt "));
     Serial.print(F("param "));
     Serial.print(interval);
     Serial.print(F(" "));
     Serial.print(F("BAT "));
     Serial.print(d);
     Serial.print(F("% "));
     Serial.print('\n');
    
  }
 }
////*******************SEQUENCE POUR CARTE FREE**********************// 
////*********appel de la fonction SIMpower pour reveiller le SIM 900************//
//      if ((Compteur1>interval)&&Compteur1<(interval+6)) {//condition
//          SIM900power(); //appel de la fonction
// }
//
////*********appel de la fonction transmission pour l envoie des messages ******//
//       if (((Compteur1>(interval+6))&&Compteur1<(interval+60))){
//           transmissionSMS (); //appel de la fonction
// }
//
////*********appel de la fonction SIMpower pour mettre en veille****************//
//       if ((Compteur1>interval+65)&&(Compteur1<interval+70)){//condition
//          SIM900power();//appel de la fonction
//          Compteur1=0;//remise a zero Compteur1
// }
////*********securitée remise a zero du compteur*********************************// 
//      if (Compteur1>=interval+120){
//         Compteur1 = 0;//remise a zero suplementaire Compteur1
// }


 //*****SEQUENCE POUR CARTE ORANGE********************//
//*********appel de la fonction SIMpower pour reveiller le SIM 900************//
      if ((Compteur1>interval)&&Compteur1<(interval+6)) {//condition
          SIM900power(); //appel de la fonction
 }

//*********appel de la fonction transmission pour l envoie des messages ******//
       if (((Compteur1>(interval+6))&&Compteur1<(interval+20))){
           transmissionSMS (); //appel de la fonction
            
 }

//*********appel de la fonction SIMpower pour mettre en veille****************//
       if ((Compteur1>interval+21)&&(Compteur1<interval+70)){//condition
          SIM900power();//appel de la fonction
          Compteur1=0;//remise a zero Compteur1
 }
//*********securitée remise a zero du compteur*********************************// 
      if (Compteur1>=interval+90){
         Compteur1 = 0;//remise a zero suplementaire Compteur1
 }
 
//**fonction selection valeurs parametre et leds visu choix interval envoie SMS**//
     switch (valpot){   
        case 20 ... 200 :
           interval=timeMsg1;//chargement valeur interval
           digitalWrite(PIN_LED_GREEN , HIGH);
           digitalWrite(PIN_LED_BLUE  , LOW );
           digitalWrite(PIN_LED_YELLOW, LOW );
           digitalWrite(PIN_LED_RED   , LOW );       
           break;
        
         case 201 ... 400 :
           interval=timeMsg2;
           digitalWrite(PIN_LED_GREEN , LOW );
           digitalWrite(PIN_LED_BLUE  , HIGH);
           digitalWrite(PIN_LED_YELLOW, LOW );
           digitalWrite(PIN_LED_RED   , LOW );   
           break;
      
         case 401 ... 600 :
           interval=timeMsg3;
           digitalWrite(PIN_LED_GREEN , LOW );
           digitalWrite(PIN_LED_BLUE  , LOW );
           digitalWrite(PIN_LED_YELLOW, HIGH);
           digitalWrite(PIN_LED_RED   , LOW );
           break;
      
        case 601 ... 800 :
           interval=timeMsg4;
           digitalWrite(PIN_LED_GREEN , LOW );
           digitalWrite(PIN_LED_BLUE  , LOW );
           digitalWrite(PIN_LED_YELLOW, LOW );
           digitalWrite(PIN_LED_RED   , HIGH);
           break;
      
         default:
           digitalWrite(PIN_LED_GREEN ,LOW );
           digitalWrite(PIN_LED_BLUE  ,LOW );
           digitalWrite(PIN_LED_YELLOW,LOW );
           digitalWrite(PIN_LED_RED   ,LOW );
           break;
    } 
   }          
         
//********** routine d'interruption du timer2*******************//
ISR (TIMER2_OVF_vect){
    // 256-6 --> 250X16uS = 4mS
    // Recharge le timer pour que la prochaine interruption se déclenche dans 4mS
    TCNT2 = 6;

    if (Compteur++ == 250) {
      //250*4mS = 1S - la Led est allumée 1 S et éteinte 1 S
      Compteur = 0;
      LedToggle;//changement d etat led Compteur1
     }
  if (Compteur == 250)
     {
      Compteur1 = Compteur1 + 1;     
     }
    }

//********fonction pour séquence de démarrage et mise en veille  du SIM 900 sur la pin 9
void SIM900power(){
  if (Compteur1>=interval){
      pinMode(9, OUTPUT);
      digitalWrite(9, LOW);
      delay(1000);
      digitalWrite(9, HIGH);
      delay(2000);
      digitalWrite(9, LOW);
      delay(3000);
      }
     }
     
//**********Fonction transmission par le SIM 900*************//
//**********declaration des commandes AT
  void transmissionSMS () {
 for ( int a = 0; a < 1; a++){// {POUR 1 NUMERO
//    for ( int a = 0; a < 2; a++){//POUR 2 NUMERO

      Serial2.println("AT");
      delay (500);
      Serial2.print("AT+CMGF=1\r"); //Activation mode Texte
      delay(1000);
      Serial2.print("AT+CMGS=\""); //Numéro du destinataire
      Serial2.println(numero[a]);
      delay(1000);
                  
      MaStructure ms_lue;//structure pour la lecture des variables en EEPROM
      EEPROM.get(0, ms_lue);//lecture des variables en EEPROM
    //Serial2.print(message);//Message à envoyer
      Serial2.write("ruche 1 ");//2eme methode pour envoyer un message
      Serial2.write("  humiditee : ");
    //Serial2.print (b);//valeur a envoyer
      Serial2.print (ms_lue.valeur_1);//valeur a envoyer
      Serial2.write("%");
      Serial2.write("  temp : "); 
    //Serial2.print (c);//valeur a envoyer
      Serial2.print (ms_lue.valeur_2);//valeur a envoyer
      Serial2.write("  poids :");
      Serial2.print(p);//valeur a envoyer //a modifier par la variable poids
      Serial2.write(" kg ");
      Serial2.write("  vitesse du vent :");
      Serial2.print(windSpeed );//valeur a envoyer 
      Serial2.write(" m/s ");  
      Serial2.write(" batterie :");
    //Serial2.print(d );//valeur a envoyer
      Serial2.print (ms_lue.valeur_3);//valeur a envoyer 
      Serial2.write("%");  
      delay(1500);
      Serial2.write((char)26); //Envoi du message
      Serial.print(a);
      delay(5000); //délai de 5 secondes entre les messages
          
    }
   }
