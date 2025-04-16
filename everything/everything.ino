#include <ArduinoBLE.h>
#include "Ultrasonic.h"

#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

#include "NRF52_MBED_TimerInterrupt.h"
#include "NRF52_MBED_ISR_Timer.h"

#define DEBUG
// #define BLACK
#define GRAY

#define FRONT_TRIGGER   4
#define FRONT_ECHO      3

enum State {
    INIT,
    ADVERTISING,
    WAITING,
    ALGORITHM,
    TIMEOUT
};
volatile State current_state = INIT;

BLEService dataService("180A");
BLEStringCharacteristic dataCharacteristic("0000AAAA-0000-1000-8000-00805F9B34FB", BLERead | BLENotify, 20); 
BLEFloatCharacteristic writeCharacteristic("0000BBBB-0000-1000-8000-00805F9B34FB", BLEWrite);

NRF52_MBED_Timer ITimer(NRF_TIMER_3);
NRF52_MBED_ISRTimer ISR_Timer;
bool advertise_timeout = 0;

Ultrasonic front(FRONT_TRIGGER, FRONT_ECHO);

void TimerHandler()
{
    ISR_Timer.run();
}


void advertisingISR()
{
    advertise_timeout = 1;
}

void setup()
{
    pinMode(LED_BUILTIN,  OUTPUT);

    #ifdef DEBUG
        Serial.begin(115200);
        while(!Serial){}
    #endif

    while(!BLE.begin()){}

    #ifdef BLACK
        BLE.setLocalName("Black Step Detector");
    #endif
    #ifdef GRAY
        BLE.setLocalName("Gray Step Detector");
    #endif
// todo: change order of these
    BLE.setAdvertisedService(dataService);
    BLE.setAdvertisingInterval(100);
    dataService.addCharacteristic(dataCharacteristic);
    dataService.addCharacteristic(writeCharacteristic);
    BLE.addService(dataService);
    BLE.advertise();
    #ifdef DEBUG
        Serial.println("BLE is advertising.");
    #endif
    current_state = ADVERTISING;
  
    if (!ITimer.attachInterruptInterval(1000, TimerHandler)) // ms
    {
        #ifdef DEBUG
            Serial.println("Timer failed to set up.");
        #endif
    }
    ISR_Timer.setInterval(10000L,  advertisingISR);
}

void loop()
{
    switch(current_state)
    {
        case ADVERTISING:
        {
            if(advertise_timeout)
            {
              BLE.stopAdvertise();
              current_state = ALGORITHM;
              break;
            }
            /* check if 10 seconds have gone by */
            
            BLEDevice app = BLE.central();
            if(app && app.connected())
            {
                #ifdef DEBUG
                    Serial.println("App is connected.");
                #endif
                current_state = WAITING;
            }
            break;
        }
        case WAITING:
        {
            #ifdef DEBUG
                Serial.println("Listening for start session.");
            #endif
            if(writeCharacteristic.written())
            {
                int number = writeCharacteristic.value();
                Serial.print("Received number: ");
                Serial.println(number);
                current_state = ALGORITHM;
            }
            break;
        }
        case ALGORITHM:
        {
          Serial.println("algo.");
          delay(1000);
            break;
        }
    }
}
/*

advertise for 10 seconds

left code and right code

if detected 

wait until I receive a start signal or I receive a calibration signal 

start > go to algorithm loop and do that (algorithm with send feature)

calibration > wait for a average step length value

if not detected > start sensing and buzzing

if no values <50 or >10 for 10 seconds, buzz for a long time because it is mis-aligned, keep buzzing until you get a good value

sensing and timing, if timer goes off, do the unlimited buzzing

 

Calibration - take first few good steps and make that target, then buzz when trending to a smaller percent

 

If no values for x amount of time AND IMU is reading movement , BUZZ

Number of times we go above threadshold
*/
