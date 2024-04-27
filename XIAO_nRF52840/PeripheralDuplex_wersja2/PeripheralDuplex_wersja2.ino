#include <ArduinoBLE.h>
#include <digitalWriteFast.h> // Wymaga instalacji biblioteki

#define LED_BLUE_PIN  LEDB
#define LED_RED_PIN   LEDR /* Same as LED_BUILTIN */
#define LED_GREEN_PIN LEDG
#define BUTTON_PIN    2

bool ledState = HIGH;
bool buttonValue = HIGH;

BLEService duplexService("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
//BLEByteCharacteristic buttonCharacteristic("473dab7c-c93a-11e9-a32f-2a2ae2dbcce4",
//    BLERead | BLENotify);
//BLEByteCharacteristic ledCharacteristic("473dacc6-c93a-11e9-a32f-2a2ae2dbcce4",
//                                        BLERead | BLEWrite);

unsigned long previousMillis = 0;
const long interval = 1000;
int interval10 = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWriteFast(LED_RED_PIN, HIGH);
  digitalWriteFast(LED_GREEN_PIN, HIGH);
  digitalWriteFast(LED_BLUE_PIN, HIGH);

  initializeBLE();
  Serial.println("Peripheral is running");
}

void initializeBLE() {
  BLE.begin();
  BLE.setLocalName("Garmin Tactix 7 Pro");
  BLE.setAdvertisedService(duplexService);

//  duplexService.addCharacteristic(ledCharacteristic);
//  duplexService.addCharacteristic(buttonCharacteristic);
  BLE.addService(duplexService);

//  ledCharacteristic.writeValue(0);
//  buttonCharacteristic.writeValue(0);

  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();
  unsigned long currentMillis = millis();

  if (central) {
    Serial.println("Got a central");

    while (central.connected()) {
      digitalWriteFast(LED_RED_PIN, HIGH);
      digitalWriteFast(LED_GREEN_PIN, LOW);

//      if (digitalRead(BUTTON_PIN) != buttonValue) {
//        buttonValue = !buttonValue;
//        buttonCharacteristic.writeValue(buttonValue);
//        Serial.println("writing to button char..");
//      }
//
//      if (ledCharacteristic.written()) {
//        ledState = ledCharacteristic.value();
//        digitalWriteFast(LED_BLUE_PIN, ledState);
//        Serial.println("changing LED");
//      }
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    digitalWriteFast(LED_RED_PIN, LOW);
    digitalWriteFast(LED_GREEN_PIN, HIGH);
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    interval10++;

    // Zmiana stanu diody
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // Ustawienie stanu diody
    digitalWriteFast(LED_RED_PIN, ledState);

    if (interval10 > 10) {
      interval10 = 0;
      Serial.println("Re-initialize the BLE hardware");
      BLE.end();
      initializeBLE();
    }

  }
}
