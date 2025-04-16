#include "StepLength.h"
#include "arduino.h"

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

float 
getStepLength(Ultrasonic device)
{
    int i = 0;
    float total_length = 0;
    float n = 0;
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
    }
    for(int j=MIN_INITIAL_VALID_READINGS; j<BURST_LENGTH; j++)
    {
        float data = device.sendPing();
        burst[j] = data;
        delay(POLL_DELAY_MS);
    }
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
