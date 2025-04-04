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

bool foo = true;
void loop() {
      while(foo)
      {
        Serial.println("front");
        float frontData = front.sendPing();
        if(frontData < 60)
        {
          Serial.print("got: ");
          Serial.println(frontData);
          delay(1000);
          foo = false;
          break;
        }
        delay(200);
      }
      while(!foo)
      {
        Serial.println("Back");
        float backData = back.sendPing();
        if(backData < 60)
        {
          Serial.print("got: ");
          Serial.println(backData);
          delay(1000);
          foo = true;
          break;
        }
        delay(200);
      }
     
  }
