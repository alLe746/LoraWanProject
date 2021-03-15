
//#define DEBUG

void setup() {
  // put your setup code here, to run once:
  SerialUSB.begin(57600);
  Serial2.begin(57600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT) ;

  while (!SerialUSB && millis() < 1000);

  LoraP2P_Setup();
  
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_GREEN, HIGH);
  char Data[100] = "";  // Array to store the message in

  if (LORA_Read(Data) == 1)
  {
    digitalWrite(LED_GREEN, LOW); // Light up LED if there is a message
    SerialUSB.println(Data);
    delay(50);
  }
}

void LoraP2P_Setup()
{

  Serial2.print("sys reset\r\n");
  delay(200);
  Serial2.print("radio set pwr ");
  Serial2.print(trPower);
  Serial2.print("\r\n");
  delay(100);
  Serial2.print("radio set sf ");
  Serial2.print(SprFactor);
  Serial2.print("\r\n");
  delay(100);
  Serial2.print("radio set wdt ");
  Serial2.print(readDelay);
  Serial2.print("\r\n");
  delay(100);
  Serial2.print("mac pause\r\n");
  delay(100);

  FlushSerialBufferIn();
}

// Setting up the receiver to read for incomming messages
void StartLoraRead()
{
  Serial2.print("radio rx 0\r\n");
  delay(100);

  FlushSerialBufferIn();
}

//////////////////////////////////////
// Read message from P2P TX module  //
// Returns 1 if there is a message  //
// Returns 2 if there is no message //
//////////////////////////////////////
int LORA_Read(char* Data)
{
  int messageFlag = 0;
  String dataStr = "radio_rx  ";
  String errorStr = "radio_err";
  String Buffer = "";

  StartLoraRead();

  while (messageFlag == 0) // As long as there is no message
  {
    while (!Serial2.available());
    
    delay(50);  // Some time for the buffer to fill

    // Read message from RN2483 LORA chip
    while (Serial2.available() > 0 && Serial2.peek() != LF)
    {
      Buffer += (char)Serial2.read();
    }

    // If there is an incoming message
    if (Buffer.startsWith(dataStr, 0)) // if there is a message in the buffer
    {
      int i = 10;  // Incoming data starts at the 11th character

      // Seperate message from string till end of datastring
      while (Buffer[i] != CR && i - 10 < max_dataSize)
      {
        Data[i - 10] = Buffer[i];
        i++;
      }
      messageFlag = 1; // Message received
    }
    else if (Buffer.startsWith(errorStr, 0))
    {
      messageFlag = 2; // Read error or Watchdogtimer timeout
    }
  }

#ifdef DEBUG
  SerialUSB.println(Buffer);
#endif

  return (messageFlag);
}

// Flushes any message available
void FlushSerialBufferIn()
{
  while (Serial2.available() > 0)
  {
#ifdef DEBUG
    SerialUSB.write(Serial2.read());
#else
    Serial2.read();
#endif
  }
}
