#include <ArduinoBLE.h>
#include "Ultrasonic.h"

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

    BLE.setLocalName("Arduino Nano BLE 33");
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
    Serial.println("Central connected.");
    while(c.connected())
    {
//      float data = back.sendPing();
      float data2 = front.sendPing();
      dataCharacteristic.writeValue(String(data2));
      Serial.print("Front: ");
      Serial.println(data2);
//      Serial.print(" Back: ");
//      Serial.println(data);
      delay(200);
    }
  }
  else
  {
//    Serial.println("Central not connected, will try again.");
  }
}
