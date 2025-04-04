#include <Adafruit_LSM6DS33.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SPI.h>
#include <math.h>
#include "Adafruit_DRV2605.h"


Adafruit_DRV2605 drv;
// Sensor instance
Adafruit_LSM6DS33 lsm6ds;

// Configuration parameters
#define SAMPLE_RATE_HZ 50  // Sampling frequency in Hz
#define STEP_THRESHOLD 10.0 // Acceleration threshold for step detection in g
#define SWING_THRESHOLD 1 // Angular velocity threshold for swing phase detection
#define MIN_STEP_TIME_MS 300 // Minimum time between steps in milliseconds

// Variables for step detection and length estimation
float accelMag = 0.0;  // Magnitude of acceleration
float gyroMag = 0.0;   // Magnitude of angular velocity
bool inSwingPhase = false;
bool inStancePhase = true;
unsigned long lastStepTime = 0;
unsigned long swingStartTime = 0;
unsigned long swingEndTime = 0;
float swingDuration = 0.0;
float stepLength = 0.0;
float totalDistance = 0.0;
int stepCount = 0;

// Calibration factors (these would need to be tuned for the specific user)
float userHeight = 170.0; // in cm
float K1 = 0.3;  // Coefficient relating swing time to step length
float K2 = 0.7;  // Coefficient relating acceleration to step length

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for serial port to connect
  
  Serial.println("Ankle-Mounted Step Length Measurement");
  
  // Initialize the LSM6DS33 sensor
  if (!lsm6ds.begin_I2C()) {
    Serial.println("Unable to initialize the LSM6DS33. Check your wiring!");
    while (1);
  }
  Serial.println("LSM6DS33 initialized successfully");
  
  // Set the sensor ranges
  lsm6ds.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  lsm6ds.setGyroRange(LSM6DS_GYRO_RANGE_500_DPS);
  
  // Set the data rates
  lsm6ds.setAccelDataRate(LSM6DS_RATE_52_HZ);
  lsm6ds.setGyroDataRate(LSM6DS_RATE_52_HZ);
}

void loop() {
  // Read the sensor data
  sensors_event_t accel, gyro, temp;
  lsm6ds.getEvent(&accel, &gyro, &temp);
  
  // Calculate magnitude of acceleration without gravity
  float accelX = accel.acceleration.x;
  float accelY = accel.acceleration.y;
  float accelZ = accel.acceleration.z - 9.8; // Remove gravity component (assuming Z is vertical)
  accelMag = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
  
  // Calculate magnitude of angular velocity
  gyroMag = sqrt(gyro.gyro.x*gyro.gyro.x + gyro.gyro.y*gyro.gyro.y + gyro.gyro.z*gyro.gyro.z);
  
  // Step detection and phase estimation algorithm
  stepDetection(accelMag, gyroMag);
  
  // Print data for debugging
  printData(accel, gyro);
  
  // Wait to maintain the desired sampling rate
  delay(1000 / SAMPLE_RATE_HZ);
}

void stepDetection(float accelMag, float gyroMag) {
  unsigned long currentTime = millis();
  
  // Detect swing phase (when the foot is off the ground)
  if (!inSwingPhase && gyroMag > SWING_THRESHOLD) {
    inSwingPhase = true;
    inStancePhase = false;
    swingStartTime = currentTime;
  }
  
  // Detect end of swing phase / beginning of stance phase
  if (inSwingPhase && gyroMag < SWING_THRESHOLD && accelMag > STEP_THRESHOLD) {
    inSwingPhase = false;
    inStancePhase = true;
    swingEndTime = currentTime;
    swingDuration = (swingEndTime - swingStartTime) / 1000.0; // in seconds
    
    // Consider it a valid step if minimum time has passed since last step
    if (currentTime - lastStepTime > MIN_STEP_TIME_MS) {
      computeStepLength();
      stepCount++;
      lastStepTime = currentTime;
    }
  }
}

void computeStepLength() {
  // Method 1: Based on swing time and user height
  float lengthFromTime = K1 * userHeight * swingDuration;
  
  // Method 2: Based on acceleration magnitude
  float lengthFromAccel = K2 * sqrt(accelMag);
  
  // Combined approach (you may want to tune this or use only one method)
  stepLength = (lengthFromTime + lengthFromAccel) / 2.0;
  
  // Update total distance
  totalDistance += stepLength;
  
  Serial.print("Step detected! Length: ");
  Serial.print(stepLength);
  Serial.print(" cm, Total: ");
  Serial.print(totalDistance);
  Serial.print(" cm, Count: ");
  Serial.println(stepCount);
  Serial.print("Method One: ");
  Serial.println(lengthFromTime);
  Serial.print("Method Two: ");
  Serial.println(lengthFromAccel);
}

void printData(sensors_event_t accel, sensors_event_t gyro) {
  //Print accelerometer data
  Serial.print("Accel: ");
  Serial.print(accel.acceleration.x); Serial.print(", ");
  Serial.print(accel.acceleration.y); Serial.print(", ");
  Serial.print(accel.acceleration.z); Serial.print(" m/s^2, Mag: ");
  Serial.print(accelMag); Serial.println(" m/s^2");
  
  // Print gyroscope data
  Serial.print("Gyro: ");
  Serial.print(gyro.gyro.x); Serial.print(", ");
  Serial.print(gyro.gyro.y); Serial.print(", ");
  Serial.print(gyro.gyro.z); Serial.print(" rad/s, Mag: ");
  Serial.print(gyroMag); Serial.println(" rad/s");
  
  // Print phase information
  Serial.print("Phase: ");
  Serial.println(inSwingPhase ? "Swing" : "Stance");
}