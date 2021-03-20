#include <RN248x_BLE.h>

#define debugSerial SerialUSB
#define bleSerial Serial1

#define SERIAL_TIMEOUT 10000  // 10 s

const char* fwVersion;

void initLed()
{
  pinMode(LED_BUILTIN, OUTPUT) ;
}

void setup() {

  initLed() ;
  
  // Wait for monitor window to be used, up to SERIAL_TIMEOUT.
  while ((!debugSerial) && (millis() < SERIAL_TIMEOUT));
  debugSerial.begin(115200);  

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
}

void loop() {

  // BT module must be put in command mode first.
  rn248xBle.enterCommandMode() ;
  // Request firmware version.
  bool res = rn248xBle.getFirmwareVersion();
  if (!res) {
    debugSerial.println("Could not get firmware version. Stopping...");
    while(1);
  }
  // At this stage, firmware version was returned by RN4871 module. Get it.
  fwVersion = rn248xBle.getLastResponse();
  if (fwVersion == NULL) {
    debugSerial.println("Firmware version can't be read. Stopping...");
    while(1);
  }
  // At this stage, we should have the firmware version.
  debugSerial.print("Firmware version: "); 
  debugSerial.println(fwVersion);
  // Now, stop.
  while(1);

}
