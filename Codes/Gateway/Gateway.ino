#include <AES.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
AES aes ;
int time;
byte key[] =
{
  0x12, 0x80, 0x02, 0x46, 0x57, 0x42, 0x13, 0x13, 0x75, 0x60, 0x44, 0x99, 0x11, 0x90, 0x07, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
} ; //key, a mettre LA MEME dans toutes les sodaq

byte cipher [4 * N_BLOCK] ; //le truc crypté à envoyer
byte check [4 * N_BLOCK] ; //le truc decrypté après la réception

void setup() {
  // put your setup code here, to run once:
  SerialUSB.begin(57600);
  Serial2.begin(57600);
  
  if (!bme.begin())  {
      SerialUSB.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

  aes.set_key (key, 256);
  // DEBUG CHECK
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT) ;

  while (!SerialUSB && millis() < 1000);

  LoraP2P_Setup();
  
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  String message="Hello1";
  byte temp[message.length()];
  message.getBytes(temp,message.length());
  aes.encrypt (temp, cipher);
  LORA_Write((char*)cipher);
  time=millis();
  while(millis()-time < 5000);
  char Data[100] = "";
  if (LORA_Read(Data)==1)
  {
    
    byte todecrypt[String(Data).length()];
    String(Data).getBytes(todecrypt,String(Data).length());
    aes.decrypt (todecrypt, check);
    SerialUSB.println(String((char *)check));
    //Envoyer en bluetooth
  }
  time=millis();
  while(millis()-time < 25000);
}
