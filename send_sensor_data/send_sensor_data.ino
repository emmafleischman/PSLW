#include <ArduinoBLE.h>
#include "ultrasonic.h"

// BLE Data Service
BLEService dataService("180A");

// BLE Characteristics
BLEStringCharacteristic dataCharacteristic("0000AAAA-0000-1000-8000-00805F9B34FB", BLERead | BLENotify, 128); // 20 is the maximum length of the string

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // turn on the built-in LED
    
    Serial.begin(115200);
    while(!Serial){}

    ultrasonic_setup();
    Serial.println("Hello, World!");

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

int i = 0;

void loop() {
  BLEDevice c = BLE.central();
  if(c && c.connected())
  {
    Serial.print("Central connected.");
    float data = ultrasonic_ping(FRONT);
    dataCharacteristic.writeValue(String(data));
    delay(1000);
  }
  else
  {
//    Serial.println("Central not connected, will try again.");
  }
}
