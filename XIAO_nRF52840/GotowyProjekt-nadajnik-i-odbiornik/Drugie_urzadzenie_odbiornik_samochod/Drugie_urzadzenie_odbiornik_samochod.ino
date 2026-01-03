#include <ArduinoBLE.h>

#define LED_BLUE_PIN LEDB
#define LED_RED_PIN  LEDR /* Same as LED_BUILTIN */
#define LED_GREEN_PIN  LEDG

// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 1000;           // interval at which to blink (milliseconds)
int standByLedState = HIGH;
int interval10 = 0;

const int optotriakPin = 8;   //PIN D7 

void setup() {
  // configure the built-in LED as an output:
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(optotriakPin, OUTPUT);

  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_BLUE_PIN, HIGH);
  digitalWrite(optotriakPin, LOW);

  Serial.begin(115200);
  //while (!Serial && millis() < 3000);

  // initialize the BLE hardware
  BLE.begin();

  // start scanning for peripherals
  BLE.scanForUuid("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
  Serial.println("Central is scanning");
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();
  digitalWrite(optotriakPin, LOW);

  if (peripheral) {
    //    digitalWrite(LED_BLUE_PIN, LOW);
    Serial.println("Got a peripheral");
    // peripheral detected, see if it's the right one:
    if (peripheral.localName() == "Garmin Tactix 7 Pro") {
      Serial.println("Got the right peripheral");
      // If so, stop scanning and start communicating with it:
      BLE.stopScan();
      communicateWith(peripheral);
    }

    // start scanning again:
    Serial.println("Start scanning again");
    BLE.scan();
    BLE.scanForUuid("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    interval10++;

    if (standByLedState == LOW) {
      standByLedState = HIGH;
    } else {
      standByLedState = LOW;
    }

    //digitalWrite(LED_BLUE_PIN, standByLedState);

    if (interval10 > 10) {
      interval10 = 0;
      Serial.println("Re-initialize the BLE hardware");
      BLE.end();
      BLE.begin();
      BLE.poll();
      BLE.scanForUuid("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
      Serial.println("Central is scanning");
    }
  }
}

void communicateWith(BLEDevice peripheral) {
  // connect to the peripheral
  // if you can't connect, go back to the main loop:
  if (!peripheral.connect()) {
    Serial.println("Can't connect");
    return;
  }
  // If you can't discover peripheral attributes
  // go back to the main loop:
  if (!peripheral.discoverAttributes()) {
    Serial.println("Didn't discover attributes");
    peripheral.disconnect();
    return;
  }

  Serial.println("Connected");
  // this while loop will run as long as you're connected to the peripheral:
  while (peripheral.connected()) {
    // turn the LED OFF RED and BLUE LED
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(LED_BLUE_PIN, HIGH);

    // turn the LED ON GREEN LED
    digitalWrite(LED_GREEN_PIN, LOW);

    // turn the D7
    digitalWrite(optotriakPin, HIGH);
  }

  peripheral.disconnect();

  // turn the LED off for good measure:
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(optotriakPin, LOW);
  Serial.println("Peripheral disconnected");
}
