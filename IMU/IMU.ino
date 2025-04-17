#include <Arduino_BMI270_BMM150.h>
//#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SPI.h>
#include <math.h>
#include "Adafruit_DRV2605.h"


Adafruit_DRV2605 drv;
// Sensor instance
//Adafruit_LSM6DS33 lsm6ds;

// Configuration parameters
#define SAMPLE_RATE_HZ 50  // Sampling frequency in Hz
#define STEP_THRESHOLD 10.0 // Acceleration threshold for step detection in g
#define SWING_THRESHOLD 23 // Angular velocity threshold for swing phase detection
#define MIN_STEP_TIME_MS 100 // Minimum time between steps in milliseconds

// Variables for step detection and length estimation
float accelX, accelY, accelZ, gryoX, gyroY, gyroZ;
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

// Define the smoothing factor (alpha)
const float alpha = 0.1; // Adjust as needed

// Initialize the smoothed value (can be a fixed value or an initial sensor reading)
float smoothedValue = 0.0;

void updateSmoothedValue(float newValue) {
  smoothedValue = (1.0 - alpha) * smoothedValue + alpha * newValue;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for serial port to connect
  
  Serial.println("Ankle-Mounted Step Length Measurement");
  
  // Init IMU
  while(!IMU.begin()){
    delay(10);
  }
  // Init Motor Driver
  while(!drv.begin()) {
    delay(10);
  }
  drv.setMode(DRV2605_MODE_INTTRIG); // default, internal trigger when sending GO command
  drv.selectLibrary(1);
  drv.setWaveform(0, 84);  // ramp up medium 1, see datasheet part 11.2
  drv.setWaveform(1, 1);  // strong click 100%, see datasheet part 11.2
  drv.setWaveform(2, 0);  // end of waveforms
}

void loop() {
  // Read the sensor data

  IMU.readAcceleration(accelX,accelY,accelZ);
  accelZ=accelZ - 9.8;
  accelMag = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
  
  // Calculate magnitude of angular velocity
  IMU.readGyroscope(gryoX, gyroY, gyroZ);
  gyroMag = sqrt(gryoX*gryoX + gyroY*gyroY + gyroZ*gyroZ);
  
  // Step detection and phase estimation algorithm
  stepDetection(accelMag, gyroMag);
  
  // Print data for debugging
  printData();
  
  // Wait to maintain the desired sampling rate
  delay(1000 / SAMPLE_RATE_HZ);
}

void stepDetection(float accelMag, float gyroMag) {
  unsigned long currentTime = millis();
  
  // Detect swing phase (when the foot is off the ground)
  if (!inSwingPhase && gyroMag > SWING_THRESHOLD && (currentTime - lastStepTime) > MIN_STEP_TIME_MS) { // If we were previously in a stance and now our gyro magnitude is higher
    inSwingPhase = true;
    inStancePhase = false;
    swingStartTime = currentTime;
    Serial.println("Now in swing phase");
  }
  
  // Detect end of swing phase / beginning of stance phase
  if (inSwingPhase && gyroMag < SWING_THRESHOLD && accelMag > STEP_THRESHOLD && (currentTime - lastStepTime) > MIN_STEP_TIME_MS ) {
    Serial.println("Now in stance phase");
    inSwingPhase = false;
    inStancePhase = true;
    swingEndTime = currentTime;
    swingDuration = (swingEndTime - swingStartTime) / 1000.0; // in seconds
    
    // Consider it a valid step if minimum time has passed since last step
    computeStepLength();
    stepCount++;
    lastStepTime = currentTime;
  }
}

void computeStepLength() {
  //Method 1: Based on swing time and user height
  float lengthFromTime = K1 * userHeight * swingDuration;
  
  // Method 2: Based on acceleration magnitude
  float lengthFromAccel = K2 * sqrt(accelMag);
  
  // drive the vibrator

  stepLength = lengthFromAccel;
  if(stepLength < 30){
    Serial.println("Step length was too short. Now going to buzz.\n");
    drv.go();
  }
  
  // Update total distance
  totalDistance += stepLength;
  
  Serial.print("Step detected! Length: ");
  Serial.println(stepLength);
  Serial.print("Step from time: ");
  Serial.println(lengthFromTime);


  //return lengthFromAccel;
}

void printData() {
  //Print accelerometer data
  // Serial.print("Accel: ");
  // Serial.print(accelMag); Serial.println(" m/s^2");
  
  // // Print gyroscope data
  // Serial.print("Gyro: ");
  // Serial.print(gyroMag); Serial.println(" rad/s");
  
  // // Print phase information
  // Serial.print("Phase: ");
  // Serial.println(inSwingPhase ? "Swing" : "Stance");
}