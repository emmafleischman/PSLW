#include "StepLength.h"
#include "arduino.h"
// #include <Arduino_BMI270_BMM150.h>
#include <Adafruit_LSM6DS33.h>

Adafruit_LSM6DS33 lsm6ds33;
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp;

float target_step_length = 0.0;
float burst[BURST_LENGTH];

void imuInit()
{
    lsm6ds33.begin_I2C();
}

float 
runCalibration(Ultrasonic *device)
{
    imuInit();
    for(int i=0; i<CALIBRATION_STEPS; i++)
    {
        float stepLen = getStepLength(device);
        while(stepLen == -1){
            stepLen = getStepLength(device);
        }
        #ifdef DEBUG
            Serial.print("Step length: ");
            Serial.println(stepLen);
        #endif
        target_step_length += stepLen;
        delay(400); 
    }
    target_step_length = target_step_length / CALIBRATION_STEPS;
    return target_step_length;
}

float 
getStepLength(Ultrasonic *device)
{
    int i = 0;
    float total_length = 0.0;
    float n = 0.0;

    unsigned long startTime = millis() ;
    // checks if we get 3 valid readings
    while(i < MIN_INITIAL_VALID_READINGS)
    {
        float data = device->sendPing();
        if(data < MAX_READING)
        {
            burst[i] = data;
            i++;
        }
        else
        {
            i = 0;
        }
        delay(POLL_DELAY_MS);
        if(millis() - startTime > NO_STEP_MAX_TIME){
            return -1;
        }
    }
    // gets the next 7 readings
    for(int j=MIN_INITIAL_VALID_READINGS; j<BURST_LENGTH; j++)
    {
        float data = device->sendPing();
        burst[j] = data;
        delay(POLL_DELAY_MS);
    }
    // removes outliers from the burst mean
    for(int k=0; k<BURST_LENGTH; k++)
    {
        if(burst[k] < MAX_READING && burst[k] != -1)
        {
            total_length += burst[k];
            n++;
        }
    }
    return total_length / n;
}

void
runAlgorithm(Ultrasonic *device, Adafruit_DRV2605 *buzzer, BLECharacteristic *stepLengthChar, bool *pause){
   float ax, ay, az, accelMagOne, accelMagTwo;
   float gx, gy, gz, gyroMagOne, gyroMagTwo;
   
   while(1)
   {
       int cumStepLen=0;
       for(int i=0; i<ALGO_WINDOW_LEN; i++){
            if(*pause)
            {
                return;
            }
           float stepLen = getStepLength(device);
           #ifdef DEBUG
           Serial.print("Got a step. Length: \n");
           Serial.println(stepLen);
           #endif

           if(stepLen < 0){ 
                #ifdef DEBUG
                Serial.println("Bad ultrasonic readings, so resorting to IMU");
                #endif
               // we haven't had a step in a while from ultrasonic! resort to IMU
               // we now want to see if there are actually steps (so check if IMU has movement). If IMU has movement over a couple windows,
               // then that means we're taking steps that are small, so we buzz.
               // if IMU has no movement, then we're just standing still, so we can continue this loop, and it'll wait for more ultrasonic readings

                lsm6ds33.getEvent(&accel, &gyro, &temp);
                ax = accel.acceleration.x;
                ay = accel.acceleration.y;
                az = accel.acceleration.z;
                gx = gyro.gyro.x;
                gy = gyro.gyro.y;
                gz = gyro.gyro.z;

               // read twice, 50 milisecond apart. if both indicate steps, then buzz. (code could be better factored)
                accelMagOne = sq(ax) + sq(ay)+sq(az);
                gyroMagOne = sq(gx) + sq(gy)+sq(gz);

                #ifdef DEBUG
                Serial.print("Reading IMU. Accel mag: ");
                Serial.print(accelMagOne);
                Serial.print(" gyro magOne: ");
                Serial.println(gyroMagOne);
                #endif

               delay(50); // arbitrary
                lsm6ds33.getEvent(&accel, &gyro, &temp);
                ax = accel.acceleration.x;
                ay = accel.acceleration.y;
                az = accel.acceleration.z;
                gx = gyro.gyro.x;
                gy = gyro.gyro.y;
                gz = gyro.gyro.z;
               accelMagTwo = sq(ax) + sq(ay)+ sq(az);
               gyroMagTwo = sq(gx) + sq(gy)+ sq(gz);

                #ifdef DEBUG
                Serial.print("Reading IMU. Accel mag: ");
                Serial.print(accelMagTwo);
                Serial.print(" gyro magtwo: ");
                Serial.println(gyroMagTwo);
                #endif

               if(accelMagOne > 92 && accelMagTwo > 92 && gyroMagOne > .05 && gyroMagTwo > 0.05){
                    buzzer->go();
                    delay(50);
                    buzzer->go();
                    if (stepLengthChar->notifyEnabled()) 
                    {
                        stepLengthChar->notify((uint8_t *)&stepLen, sizeof(float));
                    } 
               }

               cumStepLen = 32767; // hopefully the max lol
           } 
           else
           {
                if (stepLengthChar->notifyEnabled()) 
                {
                    stepLengthChar->notify((uint8_t *)&stepLen, sizeof(float));
                }
               cumStepLen += stepLen;
           }
           delay(1000);   // this represents minimum reasonable time length between steps. prolly should be like 300-500ms
        }
        float window_avg_step_length = cumStepLen / ALGO_WINDOW_LEN;
        #ifdef DEBUG
        Serial.print("window avg step length: ");
        Serial.println(window_avg_step_length);
        #endif
       
       if(PCT_THRESH*target_step_length > window_avg_step_length){
            // we want to buzz!
            #ifdef DEBUG
            Serial.print("Step length is less than the target step length, so buzzing!");
            #endif
            buzzer->go();
            float stepLen = -1;
            if (stepLengthChar->notifyEnabled()) 
            {
                stepLengthChar->notify((uint8_t *)&stepLen, sizeof(float));
            } 
       }
   }
}
