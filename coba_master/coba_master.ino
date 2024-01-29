int dir=26, en=25, pul=27; //PALING ATAS
#define stepPerRevolution 1600 //PALING ATAS

#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
uint8_t address[6]  = {0xB0, 0xA7, 0x32, 0x2A, 0xFF, 0x06};

String name = "ESP32 Slave Anakkendali.com";
char *pin = "1234"; 
bool connected;

#include <Arduino.h>
float Value;
char message;


void Step_run(){
  //KASIH BLUETOOT
//Start of (D)eploy
  //MENGULUR 
  //SETTINGAN : 1/8 -> 1600 ; 2.0-2.2A ; kel=34.55cm; 144N (50meter)  -> 230400 ; 60 RPM -> 313 ; 
  digitalWrite(dir, HIGH);//CW--Searah jarum jam -- Ulur
  for(int i=0; i<(stepPerRevolution*2.894)*2; i++){ //istilahnya steps per revolution, dikalikan x meter
    digitalWrite(pul, HIGH); //sinyal pulse high
    delayMicroseconds(313); 
    digitalWrite(pul,LOW); //sinyal pulse low 
    delayMicroseconds(313); //(1000000)/((1600)(2)(313))=1rpm
  }
  delay(5000); //5 detik ; beban ditahan holding torque

  //MENGGULUNG
  //SET : 1/8 -> 1600 ; 2.0-2.2A ; kel=34.55cm; 144N (50meter)  -> 230400 ; 60 RPM -> 313 ;
  digitalWrite(dir, LOW);//CCW--Berlawanan jarum jam -- Gulung
  for(int i=0; i<(stepPerRevolution*2.894)*2; i++){ //istilahnya steps per revolution, dikalikan x meter
    digitalWrite(pul, HIGH); //sinyal pulse high
    delayMicroseconds(313);
    digitalWrite(pul,LOW); //sinyal pulse low
    delayMicroseconds(313);
  }
  //delay(1000); //1 detik ; beban ditahan holding torque
  //End of (D)eploy 
}


void setup() {
  pinMode(dir,OUTPUT); //PALING ATAS
  pinMode(en,OUTPUT); //PALING ATAS
  pinMode(pul,OUTPUT); //PALING ATAS
  digitalWrite(en, LOW); //PALING ATAS
  Serial.begin(9600);
  SerialBT.begin("ESP32test", true); 
  Serial.println("The device started in master mode, make sure remote BT device is on!");
  
  //  connected = SerialBT.connect(name); // Jka menggunakan name tinggal samakan namanya dengan bluetooth slave tapi agak lama koneknya
    connected = SerialBT.connect(address); // lihat address slave lewat aplikasi android bluetooth
    if(connected) {
      Serial.println("Connected Succesfully!");
        Serial.println("Calculating well depth");
  SerialBT.write('G');
  Serial.println("Running stepper motor");
  Step_run();
  
    } else {
      // disconnect() may take upto 10 secs max
      if (SerialBT.disconnect()) {
        Serial.println("Disconnected Succesfully!");
      }
    SerialBT.connect();
    }
}

void loop() {

}
