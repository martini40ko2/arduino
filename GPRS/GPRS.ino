#include <SoftwareSerial.h>

SoftwareSerial GPRS(7, 8); // RX, TX

unsigned char buffer[64];
int count = 0;

void setup() {
  GPRS.begin(19200);
  Serial.begin(19200);
  delay(2000);
  //SendTextMessage();
}

void loop() {
  
  
  while (GPRS.available()) {
    buffer[count++] = GPRS.read();
    if (count == 64) break;
  }
  Serial.write(buffer, count);
  readGPRSMessage(buffer, count);

  clearBufferArray();

  if (Serial.available()) {
    byte b = Serial.read();
    if (b == '*') {
      GPRS.write(0x1a);
    } else {
      GPRS.write(b);
    }
  }
}

void clearBufferArray() {
  for (int i = 0; i < count; i++) {
    buffer[i] = NULL;
  }
  count = 0;
}

void readGPRSMessage(char wiadomosc[], int licznik){
  String message(wiadomosc);
  if(licznik > 0){
    Serial.print("TEST: ");
    Serial.print(message);
    Serial.println(" Koniec");  
  }
//  for (int i = 0; i < count; i++) {
//    buffer[i] = NULL;
//  }
}

void SendTextMessage()
{
//  GPRS.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
//  delay(100);
  GPRS.println("AT + CMGS = \"+48504266378\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  GPRS.println("A test message!");//the content of the message
  delay(100);
  GPRS.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  GPRS.println();
}

