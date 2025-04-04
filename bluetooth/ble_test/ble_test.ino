// #include <vector_type.h>
#include <ArduinoBLE.h>
#include "Ultrasonic.h"

Ultrasonic front(4, 3);

// BLE Data Service
BLEService dataService("180A");

// BLE Characteristics
BLEStringCharacteristic dataCharacteristic("0000AAAA-0000-1000-8000-00805F9B34FB", BLERead | BLENotify, 128); // 20 is the maximum length of the string
// BLEIntCharacteristic writeCharacteristic("0000BBBB-0000-1000-8000-00805F9B34FB", BLEWrite);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn on the built-in LED
  
  Serial.begin(115200);
  while(!Serial){}
  
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
//  dataService.addCharacteristic(writeCharacteristic);

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
    float data = front.sendPing(); // grabs distance in cm
    Serial.print("Central connected, sending: ");
    Serial.println(data);
    dataCharacteristic.writeValue(String(data));
    delay(200);
  }
  else
  {
//    Serial.println("Central not connected, will try again.");
  }
}
