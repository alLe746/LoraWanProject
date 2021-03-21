#include <AES.h>

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

  aes.set_key (key, 256);

  while (!SerialUSB && millis() < 1000);

  LoraP2P_Setup();
  
}

void loop() {
  String message="Hello1";
  byte temp[message.length()];
  message.getBytes(temp,message.length());
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
  char Data[100] = "";
  if (LORA_Read(Data))
  {
    String Data_str = (String)Data;
    Data_str.toLowerCase();
    Data_str.toCharArray(Data,sizeof(Data)/sizeof(Data[0]));
    byte out[(sizeof(Data)/sizeof(Data[0]))/2];
    auto getNum = [](char c){ return c > '9' ? c - 'a' + 10 : c - '0'; };
    byte *ptr = out;

    for(char *idx = Data ; *idx ; ++idx, ++ptr ){
      *ptr = (getNum( *idx++ ) << 4) + getNum( *idx );
    }
    aes.decrypt (out, check);
    //Envoyer en bluetooth
  }
  time=millis();
  while(millis()-time < 25000);
}
