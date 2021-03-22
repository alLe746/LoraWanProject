#include <AES.h>
#include <RN487x_BLE.h>

#define bleSerial Serial1
#define SERIAL_TIMEOUT  10000

const char* myDeviceName = "testPeriph" ;  // Custom Device name
const char* myPrivateServiceUUID = "AD11CF40063F11E5BE3E0002A5D5C51B" ; // Custom private service UUID
const char* temperatureCharacteristicUUID = "BF3FBD80063F11E59E690002A5D5C501" ;  // custom characteristic GATT
const uint8_t temperatureCharacteristicLen = 2 ;  // data length (in bytes)
const uint16_t temperatureHandle = 0x72 ;
char temperaturePayload[temperatureCharacteristicLen*2] ;
float temperature_s ;
uint8_t newTempValue_u8 = 0 ;
uint8_t prevTempValue_u8 = 0 ;
const char* ledCharacteristicUUID = "BF3FBD80063F11E59E690002A5D5C503" ;  // custom characteristic GATT
const uint8_t ledCharacteristicLen = 6 ;
uint16_t ledHandle = 0x75 ;
const char* ledPayload ;

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

  // Initialize the BLE hardware
  rn487xBle.hwInit() ;
  // Open the communication pipe with the BLE module
  bleSerial.begin(rn487xBle.getDefaultBaudRate()) ;
  // Assign the BLE serial port to the BLE library
  rn487xBle.initBleStream(&bleSerial) ;
  // Finalize the init. process
  rn487xBle.swInit();

  // Fist, enter into command mode
  rn487xBle.enterCommandMode() ;
  // Stop advertising before starting the demo
  rn487xBle.stopAdvertising() ;
  // Set the advertising output power (range: min = 5, max = 0)
  rn487xBle.setAdvPower(3) ;
  // Set the serialized device name, i.e. device n,ame + 2 last bytes from MAC address.
  rn487xBle.setSerializedName(myDeviceName) ;
  rn487xBle.clearAllServices() ;
  rn487xBle.reboot() ;
  rn487xBle.enterCommandMode() ;  
  // Set a private service ...
  rn487xBle.setServiceUUID(myPrivateServiceUUID) ;
  // which contains ...
  // ...a temperature characteristic; readable and can perform notification, 2-octets size
  rn487xBle.setCharactUUID(temperatureCharacteristicUUID, READ_PROPERTY | NOTIFY_PROPERTY, temperatureCharacteristicLen) ;
  // ...an LED characteristic; properties: writable
  rn487xBle.setCharactUUID(ledCharacteristicUUID, WRITE_PROPERTY, ledCharacteristicLen) ;       
  // take into account the settings by issuing a reboot
  rn487xBle.reboot() ;
  rn487xBle.enterCommandMode() ;
  // Clear adv. packet
  rn487xBle.clearImmediateAdvertising() ;
  // Start adv.
  // The line below is required, to let an Android device discover the board. 
  rn487xBle.startImmediateAdvertising(AD_TYPE_FLAGS, "06");
  rn487xBle.startImmediateAdvertising(AD_TYPE_MANUFACTURE_SPECIFIC_DATA, "CD00FE14AD11CF40063F11E5BE3E0002A5D5C51B") ;

}

void loop() {
  if (rn487xBle.getConnectionStatus())
  {
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

    rn487xBle.writeLocalCharacteristic(temperatureHandle, check)
  }
  time=millis();
  while(millis()-time < 5000);
  }
}
