/****************************************************************************************************************************
  TimerInterruptTest.ino
  For NRF52 boards using mbed-RTOS such as Nano-33-BLE
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/NRF52_MBED_TimerInterrupt
  Licensed under MIT license

  Now even you use all these new 16 ISR-based timers,with their maximum interval practically unlimited (limited only by
  unsigned long miliseconds), you just consume only one NRF52 timer and avoid conflicting with other cores' tasks.
  The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
  Therefore, their executions are not blocked by bad-behaving functions / tasks.
  This important feature is absolutely necessary for mission-critical tasks.
*****************************************************************************************************************************/
/*
   Notes:
   Special design is necessary to share data between interrupt code and the rest of your program.
   Variables usually need to be "volatile" types. Volatile tells the compiler to avoid optimizations that assume
   variable can not spontaneously change. Because your function may change variables while your program is using them,
   the compiler needs this hint. But volatile alone is often not enough.
   When accessing shared variables, usually interrupts must be disabled. Even with volatile,
   if the interrupt changes a multi-byte variable between a sequence of instructions, it can be read incorrectly.
   If your data is multiple variables, such as an array and a count, usually interrupts need to be disabled
   or the entire sequence of your code which accesses the data.
*/

#if !( ARDUINO_ARCH_NRF52840 && TARGET_NAME == ARDUINO_NANO33BLE )
#error This code is designed to run on nRF52-based Nano-33-BLE boards using mbed-RTOS platform! Please check your Tools->Board setting.
#endif

// These define's must be placed at the beginning before #include "NRF52TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
// For Nano33-BLE, don't use Serial.print() in ISR as system will definitely hang.
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
  digitalWrite(LED_BUILTIN, LOW);
  toggle0 = !toggle0;
}

void setup()
{
  pinMode(LED_BUILTIN,  OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  //  pinMode(LED_GREEN_PIN, OUTPUT);
  digitalWrite(LED_BLUE_PIN, LOW);
  Serial.begin(115200);

  while (!Serial && millis() < 5000);

  delay(100);

  Serial.print(F("\nStarting TimerInterruptTest on "));
  Serial.println(BOARD_NAME);
  Serial.println(NRF52_MBED_TIMER_INTERRUPT_VERSION);

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
