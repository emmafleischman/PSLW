//#define BLUETOOTH

#ifdef BLUETOOTH
    #include <ArduinoBLE.h>
#endif
#include "Adafruit_DRV2605.h"
#include <Adafruit_Sensor.h>
#include "Ultrasonic.h"
#include "StepLength.h"
#include <Arduino_BMI270_BMM150.h>

//#define TIMER_INTERRUPT_DEBUG         0
//#define _TIMERINTERRUPT_LOGLEVEL_     0
//
//#include "NRF52_MBED_TimerInterrupt.h"
//#include "NRF52_MBED_ISR_Timer.h"

#define DEBUG
#define BLACK
// #define GRAY

#define TRIGGER   3
#define ECHO      5

enum State {
    INIT,
    ADVERTISING,
    WAITING,
    CALIBRATION,
    ALGORITHM
};
State current_state = INIT;

Adafruit_DRV2605 buzzer;

#ifdef BLUETOOTH
BLEService dataService("180A");
BLEStringCharacteristic dataCharacteristic("0000AAAA-0000-1000-8000-00805F9B34FB", BLERead | BLENotify, 128); 
BLEFloatCharacteristic writeCharacteristic("0000BBBB-0000-1000-8000-00805F9B34FB", BLEWrite);
#endif

//NRF52_MBED_Timer ITimer(NRF_TIMER_3);
//NRF52_MBED_ISRTimer ISR_Timer;
//bool advertise_timeout = 0;

Ultrasonic front(TRIGGER, ECHO);

#ifdef BLUETOOTH
    void TimerHandler()
    {
       ISR_Timer.run();
    }
    
    void advertisingISR()
    {
       advertise_timeout = 1;
    }
#endif

void setup()
{
    #ifdef DEBUG
        Serial.begin(115200);
        while(!Serial){}
    #endif
    pinMode(LED_BUILTIN,  OUTPUT);

   // IMU setup
   if(!IMU.begin()){
       Serial.println("failed to initialize the IMU!\n");
   }else{
       Serial.println("initialized the IMU\n");
   }
    buzzer.begin();
    buzzer.setMode(DRV2605_MODE_INTTRIG); // default, internal trigger when sending GO command
    buzzer.selectLibrary(1);
    buzzer.setWaveform(0, 84);  // ramp up medium 1, see datasheet part 11.2
    buzzer.setWaveform(1, 1);  // strong click 100%, see datasheet part 11.2
    buzzer.setWaveform(2, 0);  // end of waveforms



    Serial.print("in setup\n");
    #ifdef BLUETOOTH
        while(!BLE.begin())
        {
          delay(10);
        }
       #ifdef BLACK
           BLE.setLocalName("Black Step Detector");
       #endif
       #ifdef GRAY
           BLE.setLocalName("Gray Step Detector");
       #endif
        BLE.setAdvertisedService(dataService);
        BLE.setAdvertisingInterval(100);
        dataService.addCharacteristic(dataCharacteristic);
        dataService.addCharacteristic(writeCharacteristic);
        BLE.addService(dataService);
      
        if (!ITimer.attachInterruptInterval(1000, TimerHandler)) // ms
        {
            #ifdef DEBUG
                Serial.println("Timer failed to set up.");
            #endif
        }
        ISR_Timer.setInterval(10000L,  advertisingISR);

        current_state = ADVERTISING;
    #else
        current_state = CALIBRATION;
    #endif
}

void loop()
{
    switch(current_state)
    {
        case ADVERTISING:
        {
            #ifdef BLUETOOTH
                BLE.advertise();
                #ifdef DEBUG
                    Serial.println("BLE is advertising.");
                #endif
                if(advertise_timeout)
                {
                  BLE.stopAdvertise();
                  current_state = CALIBRATION;
                  break;
                }
                
                BLEDevice c = BLE.central();
                if(c && c.connected())
                {
                    #ifdef DEBUG
                        Serial.println("App is connected.");
                    #endif
                    current_state = WAITING;
                }
            #endif
            break;
        }
        case WAITING:
        {
            #ifdef BLUETOOTH
                #ifdef DEBUG
                    Serial.println("Listening for start session.");
                #endif
                if(writeCharacteristic.written())
                {
                    float number = writeCharacteristic.value();
                    Serial.print("Received number: ");
                    Serial.println(number);
                    current_state = ALGORITHM;
                }
            #endif
            break;
        }
        case CALIBRATION:
        {
            #ifdef DEBUG
                Serial.println("Calibration Beginning.");
            #endif

            // BUZZ 3x
            buzzer.go();
            buzzer.go();
            buzzer.go();

            runCalibration(&front);

            buzzer.go();
            buzzer.go();
            buzzer.go();

            #ifdef DEBUG
                Serial.print("Calibration Complete. Target Step Length: ");
                Serial.println(target_step_length);
            #endif

            current_state = ALGORITHM;
            break;
        }
        case ALGORITHM:
        {
            #ifdef DEBUG
            #endif

            runAlgorithm(&front, &buzzer);
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
