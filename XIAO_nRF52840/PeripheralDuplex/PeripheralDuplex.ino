/*
  Peripheral duplex communication.
  This example waits for a connection from a central device.
  Once discovered and connected, the LED changes when the ledCharacteristic
  is written from by central device and the buttonCharacteristic changes
  when the local pushbutton is pressed.
  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.
  - Button connected to pin 2.
  - LED connected to built-in LED pin.
  You can use it with another board that is compatible with this library
  and running the CentralDuplex example.
  created 27 Aug 2019
  by Tom Igoe
  based on Sandeep Mistry's examples
*/

#include <ArduinoBLE.h>

#define LED_BLUE_PIN LED_BUILTIN
#define LED_RED_PIN LED_BLUE /* Same as LED_BUILTIN */
#define LED_GREEN_PIN LED_GREEN
int ledState = HIGH;
unsigned long previousMillis = 0;        // will store last time LED was updated
// constants won't change:
const long interval = 1000;           // interval at which to blink (milliseconds)
int interval10 = 0;

const int buttonPin = 2;

// create service
BLEService duplexService("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
// create button characteristic and allow remote device to get notifications
//BLEByteCharacteristic buttonCharacteristic("473dab7c-c93a-11e9-a32f-2a2ae2dbcce4",
//    BLERead | BLENotify);
// create LED characteristic and allow remote device to read and write
BLEByteCharacteristic ledCharacteristic("473dacc6-c93a-11e9-a32f-2a2ae2dbcce4",
                                        BLERead | BLEWrite);

void setup() {
  Serial.begin(115200);
  //while (!Serial);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);

  digitalWrite(LED_RED_PIN, HIGH);
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_BLUE_PIN, HIGH);

  pinMode(buttonPin, INPUT_PULLUP); // use button pin as an input

  initializeBLE();
  Serial.println("Peripheral is running");
}

void initializeBLE() {
  // begin initialization
  BLE.begin();

  // set the local name that the peripheral advertises
  BLE.setLocalName("Garmin Tactix 7 Pro");
  // set the UUID for the service the peripheral advertises:
  BLE.setAdvertisedService(duplexService);

  // add the characteristics to the service
  duplexService.addCharacteristic(ledCharacteristic);
//  duplexService.addCharacteristic(buttonCharacteristic);

  // add the service
  BLE.addService(duplexService);

  ledCharacteristic.writeValue(0);
//  buttonCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();
  unsigned long currentMillis = millis();

  if (central) {
    Serial.println("Got a central");

    while (central.connected()) {
      digitalWrite(LED_RED_PIN, HIGH);
      digitalWrite(LED_GREEN_PIN, LOW);

      // read the current button pin state
      byte buttonValue = digitalRead(buttonPin);

      // if the button has changed since the last read:
//      if (buttonCharacteristic.value() != buttonValue) {
//        // update button characteristic:
//        buttonCharacteristic.writeValue(buttonValue);
//        Serial.println("writing to button char..");
//      }

      if (ledCharacteristic.written()) {
        // update LED when central writes to characteristic:
        byte blueToothLedState = ledCharacteristic.value();
        digitalWrite(LED_BLUE_PIN, blueToothLedState);
        Serial.println("changing LED");
      }
    }

    // when the central disconnects, print it out:
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, HIGH);
  }

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    interval10++;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(LED_RED_PIN, ledState);

    if (interval10 > 10) {
      interval10 = 0;
      Serial.println("Re-initialize the BLE hardware");
      BLE.end();
      initializeBLE();
    }
  }
}
