#include <ArduinoBLE.h>
#include "ultrasonic.h"

#define FRONT_TRIGGER   4
#define FRONT_ECHO      3
#define BACK_TRIGGER    6
#define BACK_ECHO       5

Ultrasonic front;
Ultrasonic back;

// BLE Data Service
BLEService dataService("180A");

// BLE Characteristics
BLEStringCharacteristic dataCharacteristic("0000AAAA-0000-1000-8000-00805F9B34FB", BLERead | BLENotify, 128); // 20 is the maximum length of the string

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // turn on the built-in LED
    
    Serial.begin(115200);
    while(!Serial){}

    Ultrasonic front(FRONT_TRIGGER, FRONT_ECHO);
    Ultrasonic back(BACK_TRIGGER, BACK_ECHO);

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
    float data = front.sendPing();
    dataCharacteristic.writeValue(String(data));
    Serial.print("Sending data: ");
    Serial.println(data);
  }
  else
  {
//    Serial.println("Central not connected, will try again.");
  }
  delay(1000);
}
