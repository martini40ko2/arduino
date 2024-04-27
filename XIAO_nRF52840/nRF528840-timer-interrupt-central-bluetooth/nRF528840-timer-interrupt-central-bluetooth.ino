#include <ArduinoBLE.h>

#if !( ARDUINO_ARCH_NRF52840 && TARGET_NAME == ARDUINO_NANO33BLE )
#error This code is designed to run on nRF52-based Nano-33-BLE boards using mbed-RTOS platform! Please check your Tools->Board setting.
#endif

#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     3

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "NRF52_MBED_TimerInterrupt.h"

#define LED_BLUE_PIN LEDB
#define LED_RED_PIN  LEDR /* Same as LED_BUILTIN */
#define LED_GREEN_PIN  LEDG

#define TIMER0_INTERVAL_MS        1000
#define TIMER0_DURATION_MS        5000

volatile uint32_t preMillisTimer0 = 0;

static bool toggle0 = false;

// Init NRF52 timer NRF_TIMER3
NRF52_MBED_Timer ITimer0(NRF_TIMER_3);

void TimerHandler0()
{
  preMillisTimer0 = millis();

  //timer interrupt toggles pin LED_BUILTIN
  digitalWrite(LED_GREEN_PIN, LOW);

  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    Serial.println("Got a peripheral");
    // peripheral detected, see if it's the right one:
    if (peripheral.localName() == "duplexPeripheral") {
      Serial.println("Got the right peripheral");
      // If so, stop scanning and start communicating with it:
      BLE.stopScan();
      communicateWith(peripheral);
    }

    // start scanning again:
    Serial.println("Start scanning again");
    BLE.scanForUuid("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
  }
}

void communicateWith(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");
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

  // this while loop will run as long as you're connected to the peripheral:
  while (peripheral.connected()) {
    // turn the LED ON 
    digitalWrite(LED_BUILTIN, LOW); 
  }

  // turn the LED off for good measure:
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Peripheral disconnected");
}

void setup()
{
  pinMode(LED_BUILTIN,  OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);

  digitalWrite(LED_BLUE_PIN, LOW);
  Serial.begin(115200);

  while (!Serial && millis() < 5000);

  // initialize the BLE hardware
  BLE.begin();
  // start scanning for peripherals
  BLE.scanForUuid("473da924-c93a-11e9-a32f-2a2ae2dbcce4");
  Serial.println("central is scanning");


  // Interval in microsecs
  if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0)) {
    preMillisTimer0 = millis();
    Serial.print(F("Starting ITimer0 OK, millis() = "));
    Serial.println(preMillisTimer0);
  } else {
    Serial.println(F("Can't set ITimer0. Select another freq. or timer"));
  }
}

void loop()
{
  static unsigned long lastTimer0   = 0;
  static bool timer0Stopped         = false;

  if (millis() - lastTimer0 > TIMER0_DURATION_MS) {
    lastTimer0 = millis();

    if (timer0Stopped) {
      preMillisTimer0 = millis();
      Serial.print(F("Start ITimer0, millis() = "));
      Serial.println(preMillisTimer0);
      ITimer0.restartTimer();
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.print(F("Stop ITimer0, millis() = "));
      Serial.println(millis());
      ITimer0.stopTimer();
    }

    timer0Stopped = !timer0Stopped;
  }
}
