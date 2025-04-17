#include "StepLength.h"
#include "arduino.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM9DS1.h>
float target_step_length = 0.0;
float burst[BURST_LENGTH];

void 
runCalibration(Ultrasonic device)
{
    for(int i=0; i<CALIBRATION_STEPS; i++)
    {
        target_step_length += getStepLength(device);
    }
    target_step_length = target_step_length / CALIBRATION_STEPS;
}

void
runAlgorithm(Ultrasonic device){
    while(1){
        int cumStepLen=0;
        for(int i=0; i<ALGO_WINDOW_LEN; i++){
            float stepLen = getStepLength(device);
            if(stepLen < 0){ 
                // we haven't had a step in a while! resort to IMU
                // in this case our steps aren't big enough for ultrasonic to detect. 
                // we now want to see if there are actually steps (so check if IMU has movement). If IMU has movement over a couple windows,
                // then that means we're taking steps that are small, so we buzz.
                // if IMU has no movement, then we're just standing still, so we can continue this loop, and it'll wait for more ultrasonic readings


                // read twice, 50 milisecond apart. if both indicate steps, then buzz.
                lsm.read();  // this reads accel/gyro into datastructure
                float accelMagOne = sq(lsm.accelData.x) + sq(lsm.accelData.y)+sq(lsm.accelData.z);
                float gyroMagOne = sq(lsm.gyroData.x) + sq(lsm.gyroData.y)+sq(lsm.gyroData.z);
                delay(50); // arbitrary, pls fix
                lsm.read();
                float accelMagTwo = sq(lsm.accelData.x) + sq(lsm.accelData.y)+sq(lsm.accelData.z);
                float gyroMagTwo = sq(lsm.gyroData.x) + sq(lsm.gyroData.y)+sq(lsm.gyroData.z);

                if(accelMagOne > 90 && accelMagTwo > 90 && gyroMagOne > 500 && gyroMagTwo > 500){
                    buzzer.go();
                    delay(50);
                    buzzer.go();
                }


                cumStepLen = 32767; // hopefully the max lol
            }else{
                cumStepLen += getStepLength(device);
            }
        }
        if(PCT_THRESH*target_step_length > cumStepLen/ALGO_WINDOW_LEN){
            // we want to buzz!
            buzzer.go();
        }
    }
}


float 
getStepLength(Ultrasonic device)
{
    int i = 0;
    float total_length = 0;
    float n = 0;

    unsigned long startTime = millis() ;
    // checks if we get 3 valid readings
    while(i < MIN_INITIAL_VALID_READINGS)
    {
        float data = device.sendPing();
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
        float data = device.sendPing();
        burst[j] = data;
        delay(POLL_DELAY_MS);
    }
    // removes outliers from the burst mean
    for(int k=0; k<BURST_LENGTH; k++)
    {
        if(burst[k] < MAX_READING)
        {
            total_length += burst[k];
            n++;
        }
    }
    return total_length / n;
}