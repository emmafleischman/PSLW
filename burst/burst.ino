#include <ArduinoBLE.h>
#include "Ultrasonic.h"

#define FRONT_TRIGGER   4
#define FRONT_ECHO      3
#define BACK_TRIGGER    6
#define BACK_ECHO       5

Ultrasonic front(FRONT_TRIGGER, FRONT_ECHO);
Ultrasonic back(BACK_TRIGGER, BACK_ECHO);

enum State {
  EMPTY,
  ADVERTISING,
  CONNECTED,
  SENDING
};

bool session_in_progress = false;
float number = 0.0;

State current_state = EMPTY;

// BLE Data Service
BLEService dataService("180A");

// BLE Characteristics
BLEStringCharacteristic dataCharacteristic("0000AAAA-0000-1000-8000-00805F9B34FB", BLERead | BLENotify, 128); // 20 is the maximum length of the string
BLEFloatCharacteristic writeCharacteristic("0000BBBB-0000-1000-8000-00805F9B34FB", BLEWrite);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // turn on the built-in LED
    
    Serial.begin(115200);
//    while(!Serial){}

    

    // Init BLE
    while(!BLE.begin()){
        delay(10);
    }

    BLE.setLocalName("Arduino Nano BLE 33");
    BLE.setAdvertisedService(dataService);
    BLE.setAdvertisingInterval(100);

    // add the characteristics to the service
    dataService.addCharacteristic(dataCharacteristic);
    dataService.addCharacteristic(writeCharacteristic);

    // add service
    BLE.addService(dataService);

    // start advertising
    
    current_state = ADVERTISING;
}

#define BURST_SIZE 10
#define MAX_LEN 50

float burst[BURST_SIZE];
bool poll_front = true;

void print_burst()
{
  for(int i=0; i<BURST_SIZE; i++)
  {
    Serial.print(burst[i]);
    Serial.print(" ");
  }
  Serial.println(".");
}


void loop() {
      while(poll_front)
      {
        Serial.println("front");
        float frontData = front.sendPing();
        if(frontData < MAX_LEN)
        {
          burst[0] = frontData;
          
          for(int i=1; i<BURST_SIZE; i++)
          {
            float data = front.sendPing();
            burst[i] = data;
            delay(50);
          }
          Serial.print("Front: ");
          print_burst();
          // get 9 more values every 50ms
          poll_front = false;
          delay(500);
          break;
        }
        delay(200);
      }
      while(!poll_front)
      {
        Serial.println("back");
        float backData = back.sendPing();
        if(backData < MAX_LEN)
        {
          burst[0] = backData;
          for(int i=1; i<BURST_SIZE; i++)
          {
            float data = back.sendPing();
            burst[i] = data;
            delay(50);
          }
          Serial.print("Back: ");
          print_burst();
          poll_front = true;
          delay(500);
          break;
        }
        delay(200);
      }
     
  }
