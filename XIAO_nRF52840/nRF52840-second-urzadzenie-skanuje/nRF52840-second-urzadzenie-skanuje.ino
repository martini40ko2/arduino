/*
  Scan

  This example scans for Bluetooth® Low Energy peripherals and prints out their advertising details:
  address, local name, advertised service UUID's.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
const int ledPin =  LED_BUILTIN;// the number of the LED pin
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 1000;           // interval at which to blink (milliseconds)
int foundState = 0;  //if found device set on 1 and wait 1sec then scan again

void setup() {
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("Bluetooth® Low Energy Central scan");

  // start scanning for peripheral
  BLE.scan();
}

void loop() {
  if (!foundState) {
    // check if a peripheral has been discovered
    BLEDevice peripheral = BLE.available();
    if (peripheral) {
      if ("86:6e:51:14:9d:90" == peripheral.address()) {
        // turn ON led
        digitalWrite(ledPin, LOW);
        foundState = 1;
        Serial.println("Znalazlem urzadzenie.");
      }
    }

  } else {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Brak urzadzenia ...");
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      foundState = 0;

      // turn OFF led
      digitalWrite(ledPin, HIGH);
    }
  }


  //  if (peripheral) {
  //    // discovered a peripheral
  //    Serial.println("Discovered a peripheral");
  //    Serial.println("-----------------------");
  //
  //    // print address
  //    Serial.print("Address: ");
  //    Serial.println(peripheral.address());
  //
  //    // print the local name, if present
  //    if (peripheral.hasLocalName()) {
  //      Serial.print("Local Name: ");
  //      Serial.println(peripheral.localName());
  //    }
  //
  //    // print the advertised service UUIDs, if present
  //    if (peripheral.hasAdvertisedServiceUuid()) {
  //      Serial.print("Service UUIDs: ");
  //      for (int i = 0; i < peripheral.advertisedServiceUuidCount(); i++) {
  //        Serial.print(peripheral.advertisedServiceUuid(i));
  //        Serial.print(" ");
  //      }
  //      Serial.println();
  //    }
  //
  //    // print the RSSI
  //    Serial.print("RSSI: ");
  //    Serial.println(peripheral.rssi());
  //
  //    Serial.println();
  //  }
}
