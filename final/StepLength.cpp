#include "StepLength.h"
#include "arduino.h"

float target_step_length = 0.0;
float burst[BURST_LENGTH];

void 
runCalibration(Ultrasonic *device)
{
    for(int i=0; i<CALIBRATION_STEPS; i++)
    {
        target_step_length += getStepLength(device);
    }
    target_step_length = target_step_length / CALIBRATION_STEPS;
}

float 
getStepLength(Ultrasonic *device)
{
    int i = 0;
    float total_length = 0;
    float n = 0;

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
runAlgorithm(Ultrasonic *device, Adafruit_DRV2605 *buzzer){
    float ax, ay, az, accelMagOne, accelMagTwo;
    float gx, gy, gz, gyroMagOne, gyroMagTwo;
    while(1){
        int cumStepLen=0;
        for(int i=0; i<ALGO_WINDOW_LEN; i++){
            float stepLen = getStepLength(device);
            if(stepLen < 0){ 
                // we haven't had a step in a while from ultrasonic! resort to IMU
                // we now want to see if there are actually steps (so check if IMU has movement). If IMU has movement over a couple windows,
                // then that means we're taking steps that are small, so we buzz.
                // if IMU has no movement, then we're just standing still, so we can continue this loop, and it'll wait for more ultrasonic readings


                // read twice, 50 milisecond apart. if both indicate steps, then buzz. (code could be better factored)
                IMU.readAcceleration(ax, ay, az);
                IMU.readGyroscope(gx, gy, gz);
                accelMagOne = sq(ax) + sq(ay)+sq(az);
                gyroMagOne = sq(gx) + sq(gy)+sq(gz);
                delay(50); // arbitrary
                IMU.readAcceleration(ax, ay, az);
                IMU.readGyroscope(gx, gy, gz);
                accelMagTwo = sq(ax) + sq(ay)+ sq(az);
                gyroMagTwo = sq(gx) + sq(gy)+ sq(gz);

                if(accelMagOne > 90 && accelMagTwo > 90 && gyroMagOne > 500 && gyroMagTwo > 500){
                    buzzer->go();
                    delay(50);
                    buzzer->go();
                }


                cumStepLen = 32767; // hopefully the max lol
            }else{
                cumStepLen += getStepLength(device);
            }
        }
        float window_avg_step_length = cumStepLen / ALGO_WINDOW_LEN;
        
        #ifdef DEBUG
          Serial.print("window avg step length: ");
          Serial.println(window_avg_step_length);
        #endif
        
        if(PCT_THRESH*target_step_length > window_avg_step_length){
            // we want to buzz!
            buzzer->go();
        }
    }
}
