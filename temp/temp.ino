
#include <SoftwareSerial.h>

SoftwareSerial BTSerial(10, 11); // CONNECT BT RX PIN TO ARDUINO 11 PIN | CONNECT BT TX PIN TO ARDUINO 10 PIN

String command = ""; // Stores response of bluetooth device

void setup() 
{
  pinMode(9, OUTPUT);  // this pin will pull the HC-05 pin 34 (key pin) HIGH to switch module to AT mode
  digitalWrite(9, HIGH); 
  Serial.begin(9600);
  Serial.println("Enter AT commands:");
  BTSerial.begin(9600);  // HC-05 default speed in AT command more
}

void loop()
{

 if (BTSerial.available ()) // receive data if available.
  {
    while (BTSerial.available ()) // "keep receiving".
    {
      delay (10); // Delay added to make thing stable
      char c = BTSerial.read (); // Conduct serial read
      command += c; // Build the string.
    }
    Serial.println (command);
    command = ""; // No repeats
  }
  if (Serial.available ())
  {
    delay (10);
    BTSerial.write (Serial.read ());
  }
}


