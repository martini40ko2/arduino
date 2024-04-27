#include <ArduinoBLE.h>

BLEService ledService("1910000-E8F2-537E-4F6C-D104768A1214");
BLEUnsignedCharCharacteristic switchChar("1910000-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
const int ledPin =  LED_BUILTIN;// the number of the LED pin
int ledState = LOW;
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 500;           // interval at which to blink (milliseconds)

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  while (!Serial); //czeka tak długo aż odpalisz okno z monitorem portu

  if (!BLE.begin()) {
    Serial.println("Nie udało się uruchomić BLE");
    while (1);
  }

  BLE.setLocalName("MojeXIAO2");
  BLE.setAdvertisedService(ledService);
  ledService.addCharacteristic(switchChar);
  BLE.addService(ledService);

  BLE.advertise();
  Serial.println("Oczekiwanie na połączenie BLE...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    digitalWrite(ledPin, HIGH);  //turn on let after connection
    Serial.print("Połączone z: ");
    Serial.println(central.address());

    while (central.connected()) {
      switchChar.writeValue(1);
      delay(1000);
      switchChar.writeValue(0);
      delay(1000);
    }

    Serial.print("Połączone z: ");
    Serial.println(central.address());
  } else {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }

      // set the LED with the ledState of the variable:
      digitalWrite(ledPin, ledState);
    }
  }

}
