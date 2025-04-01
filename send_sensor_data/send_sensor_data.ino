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

State current_state = EMPTY;

// BLE Data Service
BLEService dataService("180A");

// BLE Characteristics
BLEStringCharacteristic dataCharacteristic("0000AAAA-0000-1000-8000-00805F9B34FB", BLERead | BLENotify, 128); // 20 is the maximum length of the string
BLEIntCharacteristic writeCharacteristic("0000BBBB-0000-1000-8000-00805F9B34FB", BLEWrite);

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
    current_state = ADVERTISING;
}

void loop() {
  switch(current_state)
  {
    case ADVERTISING:
    {
      Serial.println("in advertising state.");
      BLEDevice c = BLE.central();
      if(c && c.connected())
      {
        Serial.println("Central is connected.");
        current_state = CONNECTED;
      }
      break;
    }
    case CONNECTED:
    {
      Serial.println("Waiting to receive...");
      // wait for request to send stuff
      if(writeCharacteristic.written())
      {
        session_in_progress = writeCharacteristic.value();
        Serial.print("Received: ");
        Serial.println(session_in_progress);
        current_state = SENDING;
      }
      break;
    }
    case SENDING:
    {
      Serial.print("Sending: ");
      float data = front.sendPing();
      dataCharacteristic.writeValue(String(data));
      Serial.println(data);
      // check if we should stop
      break;
    }
  }
  delay(200);
}
