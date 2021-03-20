#include <RN2483_BLE.h>

#define debugSerial SerialUSB
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

void initLed()
{
  pinMode(LED_BUILTIN, OUTPUT) ;
}

void initTemperature()
{
  pinMode(TEMP_SENSOR, INPUT) ;
  //Set ADC resolution to 12 bits
  analogReadResolution(12) ;  
}

float getTemperature()
{
  float mVolts = (float)analogRead(TEMP_SENSOR) * 3300.0 / 1023.0 ;
  float temp = (mVolts - 500.0) / 100.0 ;
  return temp ;
}

void setup()
{
  while ((!debugSerial) && (millis() < SERIAL_TIMEOUT)) ;
  
    debugSerial.begin(115200) ;

  initLed() ;
  initTemperature() ;

  // Set the optional debug stream
  rn248xBle.setDiag(debugSerial) ;
  // Initialize the BLE hardware
  rn248xBle.hwInit() ;
  // Open the communication pipe with the BLE module
  bleSerial.begin(rn248xBle.getDefaultBaudRate()) ;
  // Assign the BLE serial port to the BLE library
  rn248xBle.initBleStream(&bleSerial) ;
  // Finalize the init. process
  if (rn248xBle.swInit())
  {
    pinMode(LED_BUILTIN, HIGH);
    debugSerial.println("Init. procedure done!") ;
  }
  else
  {
    pinMode(LED_BUILTIN, LOW);
    debugSerial.println("Init. procedure failed!") ;
    while(1) ;
  }

  // Fist, enter into command mode
  rn248xBle.enterCommandMode() ;
  // Stop advertising before starting the demo
  rn248xBle.stopAdvertising() ;
  // Set the advertising output power (range: min = 5, max = 0)
  rn248xBle.setAdvPower(3) ;
  // Set the serialized device name, i.e. device n,ame + 2 last bytes from MAC address.
  rn248xBle.setSerializedName(myDeviceName) ;
  rn248xBle.clearAllServices() ;
  rn248xBle.reboot() ;
  rn248xBle.enterCommandMode() ;  
  // Set a private service ...
  rn248xBle.setServiceUUID(myPrivateServiceUUID) ;
  // which contains ...
  // ...a temperature characteristic; readable and can perform notification, 2-octets size
  rn248xBle.setCharactUUID(temperatureCharacteristicUUID, READ_PROPERTY | NOTIFY_PROPERTY, temperatureCharacteristicLen) ;
  // ...an LED characteristic; properties: writable
  rn248xBle.setCharactUUID(ledCharacteristicUUID, WRITE_PROPERTY, ledCharacteristicLen) ;       
  // take into account the settings by issuing a reboot
  rn248xBle.reboot() ;
  rn248xBle.enterCommandMode() ;
  // Clear adv. packet
  rn248xBle.clearImmediateAdvertising() ;
  // Start adv.
  // The line below is required, to let an Android device discover the board. 
  rn248xBle.startImmediateAdvertising(AD_TYPE_FLAGS, "06");
  rn248xBle.startImmediateAdvertising(AD_TYPE_MANUFACTURE_SPECIFIC_DATA, "CD00FE14AD11CF40063F11E5BE3E0002A5D5C51B") ;

  debugSerial.println("Starter Kit as a Peripheral with private service") ;
  debugSerial.println("================================================") ;
  debugSerial.print("Private service: ") ;
  debugSerial.println(myPrivateServiceUUID) ;
  debugSerial.print("Private characteristic used for the Temperature: ") ;
  debugSerial.println(temperatureCharacteristicUUID) ;
  debugSerial.println("You can now establish a connection from the Microchip SmartDiscovery App") ;
  debugSerial.print("with the starter kit: ") ;
  debugSerial.println(rn248xBle.getDeviceName()) ;

}

void loop()
{
  // Check the connection status
  if (rn248xBle.getConnectionStatus())
  {
    // Connected to a peer
    debugSerial.print("Connected to a peer central ") ;
    debugSerial.println(rn248xBle.getLastResponse()) ;

    // Temperature
    prevTempValue_u8 = newTempValue_u8 ;
    temperature_s = getTemperature() ;
    newTempValue_u8 = (int)temperature_s ;
    // Update the local characteristic only if value has changed
    if (newTempValue_u8 != prevTempValue_u8)
    {
      uint8_t data = newTempValue_u8 ;
      temperaturePayload[3] = '0' + (data % 10) ; // LSB
      data /= 10 ;
      temperaturePayload[2] = '0' + (data % 10) ;
      data /= 10 ;
      temperaturePayload[1] = '0' + (data % 10) ;
      if (temperature_s > 0)  temperaturePayload[0] = '0' ; // MSB = 0, positive temp.
      else                    temperaturePayload[0] = '8' ; // MSB = 1, negative temp.  
         
      if (rn248xBle.writeLocalCharacteristic(temperatureHandle, temperaturePayload))
      {
        debugSerial.print("Temperature characteristic has been updated with the value = ") ; debugSerial.println(newTempValue_u8) ;
      }
    }        
  }
  else
  {
    // Not connected to a peer device
    debugSerial.println("Not connected to a peer device") ;
  }
  // Delay inter connection polling
  delay(3000) ;
}
