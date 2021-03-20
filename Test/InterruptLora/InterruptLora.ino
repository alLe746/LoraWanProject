void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
attachInterrupt(34,exampleISR,CHANGE);
}
int a=0;
void loop() {
  // put your main code here, to run repeatedly:
Serial.println(a);
}
void exampleISR()
{
  //Normally it is bad practice
  //to use a println() in an ISR
  a=1;
  noInterrupts();
}
