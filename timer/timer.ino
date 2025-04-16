// These define's must be placed at the beginning before #include "NRF52TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
// For Nano33-BLE, don't use Serial.print() in ISR as system will definitely hang.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "NRF52_MBED_TimerInterrupt.h"

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "NRF52_MBED_ISR_Timer.h"

//#ifndef LED_BUILTIN
//  #define LED_BUILTIN         D13
//#endif

#ifndef LED_BLUE_PIN
  #if defined(LEDB)
    #define LED_BLUE_PIN          LEDB
  #else
    #define LED_BLUE_PIN          D7
  #endif
#endif

#ifndef LED_RED_PIN
  #if defined(LEDR)
    #define LED_RED_PIN           LEDR
  #else
    #define LED_RED_PIN           D8
  #endif
#endif

#define HW_TIMER_INTERVAL_MS      1

// For core mbed core 1.3.2-
// Depending on the board, you can select NRF52 Hardware Timer from NRF_TIMER_1,NRF_TIMER_3,NRF_TIMER_4 (1,3 and 4)
// If you select the already-used NRF_TIMER_0 or NRF_TIMER_2, it'll be auto modified to use NRF_TIMER_1

// For core mbed core 2.0.0-
// Depending on the board, you can select NRF52 Hardware Timer from NRF_TIMER_3,NRF_TIMER_4 (3 and 4)
// If you select the already-used NRF_TIMER_0, NRF_TIMER_1 or NRF_TIMER_2, it'll be auto modified to use NRF_TIMER_3

// Init NRF52 timer NRF_TIMER3
NRF52_MBED_Timer ITimer(NRF_TIMER_3);

// Init NRF52_MBED_ISRTimer
// Each NRF52_MBED_ISRTimer can service 16 different ISR-based timers
NRF52_MBED_ISRTimer ISR_Timer;

#define TIMER_INTERVAL_1S             1000L
#define TIMER_INTERVAL_2S             2000L
#define TIMER_INTERVAL_5S             5000L

void TimerHandler()
{
  ISR_Timer.run();
}

int flag = 0;

// In NRF52, avoid doing something fancy in ISR, for example complex Serial.print with String() argument
// The pure simple Serial.prints here are just for demonstration and testing. Must be eliminate in working environment
// Or you can get this run-time error / crash
void doingSomething1()
{
  flag = 1;
}

// void doingSomething2()
// {
//   digitalWrite(LED_BLUE_PIN, !digitalRead(LED_BLUE_PIN));
// }
// void doingSomething3()
// {
//   digitalWrite(LED_RED_PIN, !digitalRead(LED_RED_PIN));
// }

void setup()
{
  Serial.begin(115200);

  while (!Serial && millis() < 5000);

  delay(100);

  Serial.print(F("\nStarting TimerInterruptLEDDemo on "));
  Serial.println(BOARD_NAME);
  Serial.println(NRF52_MBED_TIMER_INTERRUPT_VERSION);

  // configure pin in output mode
  pinMode(LED_BUILTIN,  OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(LED_RED_PIN,  OUTPUT);

  // Interval in microsecs
  if (!ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS * 1000, TimerHandler))
  {
    Serial.println("timer failed");
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));

  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  // You can use up to 16 timer for each ISR_Timer
  ISR_Timer.setInterval(TIMER_INTERVAL_1S,  doingSomething1);
}


void loop()
{
  if(flag)
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    flag = 0;
  }
  /* Nothing to do all is done by hardware. Even no interrupt required. */
}