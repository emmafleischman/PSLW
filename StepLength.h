#ifndef _STEP_LENGTH_H_
#define _STEP_LENGTH_H_

//#include "Arduino_BMI270_BMM150.h"
#include "Adafruit_DRV2605.h"
#include "Ultrasonic.h"

#define BURST_LENGTH                10
#define MIN_INITIAL_VALID_READINGS  3
#define MAX_READING                 50 
#define CALIBRATION_STEPS           5
#define POLL_DELAY_MS               10
#define ALGO_WINDOW_LEN              4
const float PCT_THRESH = 0.8;
#define NO_STEP_MAX_TIME          5000

extern float target_step_length;
extern float burst[];

void runCalibration(Ultrasonic *device);
float getStepLength(Ultrasonic *device);
void runAlgorithm(Ultrasonic *device, Adafruit_DRV2605 *buzzer);

#endif
