// #include <vector_type.h>
#include <ArduinoBLE.h>

// BLE Data Service
BLEService dataService("180A");

// BLE Characteristics
// BLEStringCharacteristic dataCharacteristic("0000AAAA-0000-1000-8000-00805F9B34FB", BLERead | BLENotify, 128); // 20 is the maximum length of the string
BLEIntCharacteristic writeCharacteristic("0000BBBB-0000-1000-8000-00805F9B34FB", BLEWrite);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn on the built-in LED
  
  Serial.begin(115200);

  // Init BLE
  while(!BLE.begin()){
    delay(10);
  }

  BLE.setLocalName("Emma");
  BLE.setAdvertisedService(dataService);
  BLE.setAdvertisingInterval(100);

  // add the characteristics to the service
  dataService.addCharacteristic(dataCharacteristic);
  dataService.addCharacteristic(writeCharacteristic);

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
    Serial.println("Central connected, sending: %d", i);
    // Send message to central
    dataCharacteristic.writeValue(i);
    i++;
    delay(1000);
  }
  else
  {
    Serial.println("Central not connected, will try again.");
  }
}
