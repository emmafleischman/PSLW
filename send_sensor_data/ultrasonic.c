#include <Arduino.h>
#include "ultrasonic.h"

void ultrasonic_setup()
{
    pinMode(frontTrig, OUTPUT);
    pinMode(frontEcho, INPUT);
    pinMode(backTrig, OUTPUT);
    pinMode(backEcho, INPUT);
}

float ultrasonic_ping(SensorPosition pos)
{
    switch(pos)
    {
        case FRONT:
            digitalWrite(frontTrig, HIGH);
            delayMicroseconds(10);
            digitalWrite(frontTrig, LOW);
            long frontDuration = pulseIn(frontEcho, HIGH, 1000000);
            float frontDistance = frontDuration * 0.034 / 2;
            return frontDistance;
        case BACK:
            digitalWrite(backTrig, HIGH);
            delayMicroseconds(10);
            digitalWrite(backTrig, LOW);
            long backDuration = pulseIn(backEcho, HIGH, 1000000);
            float backDistance = backDuration * 0.034 / 2;
            return backDistance;
        default:
            return -1;
    }
}
