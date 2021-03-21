#include <AES.h>

//#define DEBUG

#include <AES.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
AES aes ;
String Keyword = "Hello";
byte key[] =
{
  0x12, 0x80, 0x02, 0x46, 0x57, 0x42, 0x13, 0x13, 0x75, 0x60, 0x44, 0x99, 0x11, 0x90, 0x07, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
} ; //key, a mettre LA MEME dans toutes les sodaq

byte cipher [4 * N_BLOCK] ; //le truc crypté à envoyer
byte check [4 * N_BLOCK] ; //le truc decrypté après la réception

void setup() {

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
  
}

void loop() {
  
  char Data[100] = "";  // Array to store the message in
  SerialUSB.println("got loop ?");
  if (LORA_Read(Data))
  {
    SerialUSB.println("Data :");
    String Data_str = (String)Data;
    Data_str.toLowerCase();
    Data_str.toCharArray(Data,sizeof(Data)/sizeof(Data[0]));
    byte out[(sizeof(Data)/sizeof(Data[0]))/2];
    auto getNum = [](char c){ return c > '9' ? c - 'a' + 10 : c - '0'; };
    byte *ptr = out;

    for(char *idx = Data ; *idx ; ++idx, ++ptr ){
      *ptr = (getNum( *idx++ ) << 4) + getNum( *idx );
    }
    aes.decrypt(out,check);
    if ((String)(char *)check == Keyword)
    {
      String value = String(bme.readTemperature());
      byte temp[value.length()];
      value.getBytes(temp,value.length());
      
      aes.encrypt (temp, cipher);
      String Tosend = "";
      for (byte i = 0 ; i < sizeof(cipher)/sizeof(cipher[0]) ; i++)
      {
        byte val = cipher[i];
        Tosend += String(val >> 4, HEX) ; Tosend += String(val & 15, HEX) ;
      }
      char * Tosendchar = new char[Tosend.length() +1];
      strcpy(Tosendchar,Tosend.c_str());
      LORA_Write(Tosendchar);
      digitalWrite(LED_GREEN, HIGH); // To let us know when the data is send
      delay(1000);
      digitalWrite(LED_GREEN, LOW);
      delay(50);
    }
  }
}
