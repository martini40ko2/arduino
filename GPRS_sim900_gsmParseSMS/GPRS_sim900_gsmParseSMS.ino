/*
 *  Polaczenie arduino UNO z SIM900
 *  Arduino   ->  Sim900
 *  PIN7      ->  PIN0 TX
 *  PIN8      ->  PIN1 RX
 *  GND       ->  GND (sa dokladnie w takich samych miejscach jak na plytce arduino uno)
 *            ->  Vcc - zasilacz zewnetrzny DC12V
 * 
 */
#include <SoftwareSerial.h>
SoftwareSerial GPRS(7, 8); // ustawienie software serial dla arduino RX, TX

enum _parseState {
  PS_DETECT_MSG_TYPE,

  PS_IGNORING_COMMAND_ECHO,

  PS_READ_CMTI_STORAGE_TYPE,
  PS_READ_CMTI_ID,

  PS_READ_CMGR_STATUS,
  PS_READ_CMGR_NUMBER,
  PS_READ_CMGR_SOMETHING,
  PS_READ_CMGR_DATE,
  PS_READ_CMGR_CONTENT
};

byte state = PS_DETECT_MSG_TYPE;

char buffer[80];
byte pos = 0;

int lastReceivedSMSId = 0;
boolean validSender = false;
String receiverNumber;

boolean ledEnabled = false;
boolean pin2Enabled = false;
boolean pin4Enabled = false;

void resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

void setup()
{
  GPRS.begin(19200);
  Serial.begin(19200);

  // Set as appropriate for your case
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(4, HIGH);

  Serial.println("Initializacja modemu.");
  GPRS.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode

  Serial.println("Usuwanie starych wiadomosci SMS.");
  for (int i = 1; i <= 25; i++) {
    GPRS.print("AT+CMGD=");
    GPRS.println(i);
    delay(200);

    // Not really necessary but prevents the serial monitor from dropping any input
    //    while (GPRS.available()) {
    //      Serial.write(GPRS.read());
    //    }
  }
  Serial.println("Inicjalizacja zakonczona.");
}

void loop()
{
  while (GPRS.available()) {
    parseATText(GPRS.read());
  }
}

/*
   Parsowanie polecen AT znak po znaku
   np:
       +CMTI: "SM",1
       +CMGR: "REC UNREAD","+48504266378","","17/12/27,21:57:10+04"
*/
void parseATText(byte b) {

  buffer[pos++] = b;

  if ( pos >= sizeof(buffer) )
    resetBuffer(); // just to be safe

  /*
    // Detailed debugging
    Serial.println();
    Serial.print("state = ");
    Serial.println(state);
    Serial.print("b = ");
    Serial.println(b);
    Serial.print("pos = ");
    Serial.println(pos);
    Serial.print("buffer = ");
    Serial.println(buffer);*/

  switch (state) {
    case PS_DETECT_MSG_TYPE:
      {
        if ( b == '\n' ) {  //ignoruj puste linie
          resetBuffer();
        } else {
          if ( pos == 3 && strcmp(buffer, "AT+") == 0 ) { //jesli komenda zaczyna sie od AT+ to tez ignoruj
            state = PS_IGNORING_COMMAND_ECHO;
          } else if ( pos == 6 ) {
            //Serial.print("Checking message type: ");
            //Serial.println(buffer);

            if ( strcmp(buffer, "+CMTI:") == 0 ) {
              Serial.println("Received CMTI");
              state = PS_READ_CMTI_STORAGE_TYPE;
            }
            else if ( strcmp(buffer, "+CMGR:") == 0 ) {
              Serial.println("Received CMGR");
              state = PS_READ_CMGR_STATUS;
            }
            resetBuffer();
          }
        }
      }
      break;

    case PS_IGNORING_COMMAND_ECHO:
      {
        if ( b == '\n' ) {
          //Serial.print("Ignoring echo: ");
          //Serial.println(buffer);
          state = PS_DETECT_MSG_TYPE;
          resetBuffer();
        }
      }
      break;

    case PS_READ_CMTI_STORAGE_TYPE:
      {
        if ( b == ',' ) {
          Serial.print("SMS storage is ");
          Serial.println(buffer);
          state = PS_READ_CMTI_ID;
          resetBuffer();
        }
      }
      break;

    case PS_READ_CMTI_ID:
      {
        if ( b == '\n' ) {
          lastReceivedSMSId = atoi(buffer);
          Serial.print("SMS id is ");
          Serial.println(lastReceivedSMSId);

          GPRS.print("AT+CMGR=");
          GPRS.println(lastReceivedSMSId);
          //delay(500); don't do this!

          state = PS_DETECT_MSG_TYPE;
          resetBuffer();
        }
      }
      break;

    case PS_READ_CMGR_STATUS:
      {
        if ( b == ',' ) {
          Serial.print("CMGR status: ");
          Serial.println(buffer);
          state = PS_READ_CMGR_NUMBER;
          resetBuffer();
        }
      }
      break;

    case PS_READ_CMGR_NUMBER:
      {
        if ( b == ',' ) {
          Serial.print("CMGR number: ");
          Serial.println(buffer);
          

          // Uncomment these two lines to check the sender's cell number
          //validSender = false;
          //if ( strcmp(buffer, "\"+0123456789\",") == 0 )
          validSender = true;

          receiverNumber = buffer;  //zapisuje numer nadawcy ostatnie wiadomosci w zmiennej receiverNumber

          state = PS_READ_CMGR_SOMETHING;
          resetBuffer();
        }
      }
      break;

    case PS_READ_CMGR_SOMETHING:
      {
        if ( b == ',' ) {
          Serial.print("CMGR something: ");
          Serial.println(buffer);
          state = PS_READ_CMGR_DATE;
          resetBuffer();
        }
      }
      break;

    case PS_READ_CMGR_DATE:
      {
        if ( b == '\n' ) {
          Serial.print("CMGR date: ");
          Serial.println(buffer);
          state = PS_READ_CMGR_CONTENT;
          resetBuffer();
        }
      }
      break;

    case PS_READ_CMGR_CONTENT:
      {
        if ( b == '\n' ) {
          Serial.print("CMGR content: ");
          Serial.print(buffer);

          parseSMSContent();

          GPRS.print("AT+CMGD=");
          GPRS.println(lastReceivedSMSId);
          //delay(500); don't do this!

          state = PS_DETECT_MSG_TYPE;
          resetBuffer();
        }
      }
      break;
  }
}
/*
   Odczytywanie kontentu wiadomosci
*/
void parseSMSContent() {

  char* ptr = buffer;

  while ( strlen(ptr) >= 2 ) {

    //jesli wiadomosc zaczyna sie od 'r' lub 'R'
    if ( ptr[0] == 'r' ||  ptr[0] == 'R' ) {
      if ( ptr[1] == '1' ) {
        pin2Enabled = true;
        digitalWrite(2, HIGH);
      } else {
        pin2Enabled = false;
        digitalWrite(2, LOW);
      }
    }

    //jesli wiadomosc zaczyna sie od 'y' lub 'Y'
    if ( ptr[0] == 'y' ||  ptr[0] == 'Y' ) {
      if ( ptr[1] == '1' ) {
        pin4Enabled = true;
        digitalWrite(4, HIGH);
      } else {
        pin4Enabled = false;
        digitalWrite(4, LOW);
      }
    }

    //jesli wiadomosc zaczyna sie od 'l' lub 'L'
    if ( ptr[0] == 'l' ||  ptr[0] == 'L') {
      if ( ptr[1] == '1' ) {
        ledEnabled = true;
        digitalWrite(LED_BUILTIN, HIGH);
      } else {
        ledEnabled = false;
        digitalWrite(LED_BUILTIN, LOW);
      }
    }

    //jesli wiadomosc zaczyna sie od 's' lub 'S' - status
    if ( ptr[0] == 's' ||  ptr[0] == 'S') {
      Serial.println("Wysylam status do: "+ receiverNumber.substring(0,14));
      sendTextMessage(receiverNumber.substring(0,14));
    }

    ptr += 2;
  }
}

void sendTextMessage(String phoneNumber)
{
  String pin2Status = pin2Enabled ? "ON" : "OFF";
  String pin4Status = pin4Enabled ? "ON" : "OFF";
  String ledStatus = ledEnabled ? "ON" : "OFF";
  
  String message = "PIN2 is " + pin2Status + "\n";
  message = message + "PIN4 is " + pin4Status + "\n";
  message = message + "LED  is " + ledStatus;

  Serial.println(message);
  GPRS.println("AT+CMGS="+phoneNumber); //send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  GPRS.println(message);//the content of the message
  delay(100);
  GPRS.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  GPRS.println();
}


