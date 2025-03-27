#include <Arduino.h>
#include "ultrasonic.h"

Ultrasonic::Ultrasonic(){};

Ultrasonic::Ultrasonic(int frontTrig, int frontEcho, int backTrig, int backEcho)
: m_front_trig{frontTrig}, m_front_echo{frontEcho}, m_back_trig{backTrig}, m_back_echo{backEcho}
{
    pinMode(frontTrig, OUTPUT);
    pinMode(frontEcho, INPUT);
    pinMode(backTrig, OUTPUT);
    pinMode(backEcho, INPUT);
}

float Ultrasonic::sendPing(SensorPosition pos)
{
    float distance;
    long frontDuration;
    long backDuration;
    switch(pos)
    {
        case FRONT:
            digitalWrite(m_front_trig, HIGH);
            delayMicroseconds(10);
            digitalWrite(m_front_trig, LOW);
            frontDuration = pulseIn(m_front_echo, HIGH, 1000000);
            distance = frontDuration * 0.034 / 2;
            break;
        case BACK:
            digitalWrite(m_back_trig, HIGH);
            delayMicroseconds(10);
            digitalWrite(m_back_trig, LOW);
            backDuration = pulseIn(m_back_echo, HIGH, 1000000);
            distance = backDuration * 0.034 / 2;
            break;
        default:
            distance = -1;
            break;
        return distance;
    }
}
