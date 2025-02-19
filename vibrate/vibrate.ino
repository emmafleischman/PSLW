#include <ReefwingAHRS.h>
#include <Arduino_BMI270_BMM150.h>
#include <vector_type.h>
#include "Filter.h"
#include "Integral.h"
#include <ArduinoBLE.h>
#include "Adafruit_DRV2605.h"

Adafruit_DRV2605 drv;

// BLE Data Service
BLEService dataService("180A");

// Left Device
BLEStringCharacteristic dataCharacteristic("0000AAAA-0000-1000-8000-00805F9B34FB", BLERead | BLENotify, 128); // 20 is the maximum length of the string
BLEFloatCharacteristic writeCharacteristic("0000BBBB-0000-1000-8000-00805F9B34FB", BLEWrite);

// Right Device
//BLEStringCharacteristic dataCharacteristic("2A57", BLERead | BLENotify, 128);
//BLEFloatCharacteristic writeCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite);

SensorData data;
ReefwingAHRS ahrs;

Filter accelFilter, gyroFilter;
Integral velIntegral, posIntegral;
DeltaTime dt;
ZVU zvu;

#define GRAVITY 9.81
#define ACCEL_THRESH 1
#define GYRO_THRESH 3.0
#define MIN_STEP_LENGTH 0.02
#define MAX_STEP_LENGTH 5.00
#define M_TO_CM 100

char input = 'F';

float step_cum = 0;
float step_target = 0.50;

bool connectBLE = true;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn on the built-in LED
  
  ahrs.begin();

  accelFilter.begin(5, 119); 
  gyroFilter.begin(5, 119);
  zvu.begin(ACCEL_THRESH, GYRO_THRESH, 12);
  
  ahrs.setFusionAlgorithm(SensorFusion::MAHONY);
  ahrs.setDeclination(-9.25059);
  ahrs.setKp(40.0);
  ahrs.setKi(0.0);
//  ahrs.setBeta(0);
//  ahrs.setAlpha(0.95);
  Serial.begin(115200);

  // Init Motor Driver
  while(!drv.begin()) {
    delay(10);
  }
  
  drv.setMode(DRV2605_MODE_INTTRIG); // default, internal trigger when sending GO command
  drv.selectLibrary(1);
  drv.setWaveform(0, 84);  // ramp up medium 1, see datasheet part 11.2
  drv.setWaveform(1, 1);  // strong click 100%, see datasheet part 11.2
  drv.setWaveform(2, 0);  // end of waveforms

  // Init BLE
  while(!BLE.begin()){
    delay(10);
  }
//  BLE.setLocalName("Right Step Detector");
  BLE.setLocalName("Left Step Detector");
  BLE.setAdvertisedService(dataService);
  BLE.setAdvertisingInterval(100);

  // add the characteristics to the service
  dataService.addCharacteristic(dataCharacteristic);
  dataService.addCharacteristic(writeCharacteristic);

  // add service
  BLE.addService(dataService);

  // start advertising
  BLE.advertise();
    Serial.println("BLE Started.");

  // Init IMU
  while(!IMU.begin()){
    delay(10);
  }

  Serial.println("Ready!!!\n");

  while(1) {
//    if(Serial.available()){
//      input = readChar();
//    }
//    BLEDevice central;
////    central = BLE.central();
//    if(connectBLE) bleFetch();
    updateLoop();
  }
  
}

void minUpdate(){
  if (IMU.gyroscopeAvailable()) {  
    IMU.readGyroscope(data.gx, data.gy, data.gz);
    data.gTimeStamp = micros();
  }
  if (IMU.accelerationAvailable()) {  
    IMU.readAcceleration(data.ax, data.ay, data.az);  
    data.aTimeStamp = micros();
  }
  if (IMU.magneticFieldAvailable()) {
    IMU.readMagneticField(data.mx, data.my, data.mz);
    data.mTimeStamp = micros();
  }
  ahrs.setData(data); //data is a struct {ax,ay,az,gx,gy,gz,mx,my,mz,gTimeStamp,aTimeStamp,mTimeStamp}
  ahrs.update();
}

void updateLoop(){
  Serial.println(".");
  digitalWrite(LED_BUILTIN, LOW);
  vibrate();
  delay(1000);
}

vec3_t getGravity(Quaternion q){
  float r0 = 2*(q.q1*q.q3-q.q0*q.q2);
  float r1 = 2*(q.q2*q.q3+q.q0*q.q1);
//  float r2 = 2*(q.q0*q.q0+q.q3*q.q3)-1;
  float r2 = q.q0*q.q0 - q.q1*q.q1 - q.q2*q.q2 + q.q3*q.q3;
  return {r0, r1, r2};
}

vec3_t projectAccelOnGravity(vec3_t a, vec3_t g){
  return a - a.dot(g)*g;
}

vec3_t subtractGravity(vec3_t a, vec3_t g){
  return a - g;
}

vec3_t calculatePos(vec3_t a, uint32_t ts){
  float delta = dt.step(data.aTimeStamp);
  return posIntegral.step(velIntegral.step(a, delta), delta);
}

void resetPos(){
  velIntegral.reset(getAccel(data));
  posIntegral.reset({0, 0, 0});
  dt.set(data.aTimeStamp);
}

void resetVelocity() {
  velIntegral.reset(getAccel(data));
  posIntegral.resetPrev({0, 0, 0});
  dt.set(data.aTimeStamp);
}

void vibrate(){
  drv.go();
}

void loop() {}
