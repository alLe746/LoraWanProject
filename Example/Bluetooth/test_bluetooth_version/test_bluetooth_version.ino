#include <RN487x_BLE.h>

#define debugSerial SerialUSB
#define bleSerial Serial1

#define SERIAL_TIMEOUT 10000  // 10 s

const char* fwVersion;

void setRgbColor(uint8_t red, uint8_t green, uint8_t blue)
{
  red = 255 - red ;
  green = 255 - green ;
  blue = 255 - blue ;

  analogWrite(LED_RED, red) ;
  analogWrite(LED_GREEN, green) ;
  analogWrite(LED_BLUE, blue) ;
}

void setup() {

  // Wait for monitor window to be used, up to SERIAL_TIMEOUT.
  while ((!debugSerial) && (millis() < SERIAL_TIMEOUT));
  debugSerial.begin(115200);  

  // Set the optional debug stream
  rn487xBle.setDiag(debugSerial) ;
  // Initialize the BLE hardware
  rn487xBle.hwInit() ;
  // Open the communication pipe with the BLE module
  bleSerial.begin(rn487xBle.getDefaultBaudRate()) ;
  // Assign the BLE serial port to the BLE library
  rn487xBle.initBleStream(&bleSerial) ;
  // Finalize the init. process
  if (rn487xBle.swInit())
  {
    setRgbColor(0, 255, 0) ;
    debugSerial.println("Init. procedure done!") ;
  }
  else
  {
    setRgbColor(255, 0, 0) ;
    debugSerial.println("Init. procedure failed!") ;
    while(1) ;
  }

}

void loop() {

  // BT module must be put in command mode first.
  rn487xBle.enterCommandMode() ;
  // Request firmware version.
  bool res = rn487xBle.getFirmwareVersion();
  if (!res) {
    debugSerial.println("Could not get firmware version. Stopping...");
    while(1);
  }
  // At this stage, firmware version was returned by RN4871 module. Get it.
  fwVersion = rn487xBle.getLastResponse();
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
