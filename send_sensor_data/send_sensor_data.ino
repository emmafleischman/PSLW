#include <ArduinoBLE.h>
#include "ultrasonic.h"

#define FRONT_TRIGGER   4
#define FRONT_ECHO      3
#define BACK_TRIGGER    6
#define BACK_ECHO       5

Ultrasonic front(FRONT_TRIGGER, FRONT_ECHO);
Ultrasonic back(BACK_TRIGGER, BACK_ECHO);

// BLE Data Service
BLEService dataService("180A");

// BLE Characteristics
BLEStringCharacteristic dataCharacteristic("0000AAAA-0000-1000-8000-00805F9B34FB", BLERead | BLENotify, 128); // 20 is the maximum length of the string

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // turn on the built-in LED
    
    Serial.begin(115200);
    while(!Serial){}

    

    // Init BLE
    while(!BLE.begin()){
        delay(10);
    }

    BLE.setLocalName("ultrasonic system");
    BLE.setAdvertisedService(dataService);
    BLE.setAdvertisingInterval(100);

    // add the characteristics to the service
    dataService.addCharacteristic(dataCharacteristic);

    // add service
    BLE.addService(dataService);

    // start advertising
    BLE.advertise();
    Serial.println("BLE is Advertising.");
}

void loop() {
  BLEDevice c = BLE.central();
  if(c && c.connected())
  {
    Serial.print("Central connected.");
    while(1)
    {
      float data = back.sendPing();
      float data2 = front.sendPing();
      dataCharacteristic.writeValue(String(data));
      Serial.print("Front: ");
      Serial.print(data2);
      Serial.print(" Back: ");
      Serial.println(data);
      delay(2000);
    }
  }
  else
  {
//    Serial.println("Central not connected, will try again.");
  }
}
