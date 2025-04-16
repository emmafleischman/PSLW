#ifndef _STEP_LENGTH_H_
#define _STEP_LENGTH_H_

#include "Ultrasonic.h"

#define BURST_LENGTH                10
#define MIN_INITIAL_VALID_READINGS  3
#define MAX_READING                 50 
#define CALIBRATION_STEPS           5
#define POLL_DELAY_MS               10

extern float target_step_length;
extern float burst[];

void runCalibration(Ultrasonic device);
float getStepLength(Ultrasonic device);

#endif