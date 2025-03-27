#ifndef _ULTRASONIC_H_
#define _ULTRASONIC_H_

#define frontTrig    4
#define frontEcho    3
#define backTrig     6
#define backEcho     5

typedef enum {
    FRONT, 
    BACK
} SensorPosition;

extern void ultrasonic_setup();
extern float ultrasonic_ping(SensorPosition pos);

 #endif
