#include <Arduino.h>
#include "Ultrasonic.h"

Ultrasonic::Ultrasonic(){};

Ultrasonic::Ultrasonic(int trigger, int echo)
: m_trigger{trigger}, m_echo{echo}
{
    pinMode(trigger, OUTPUT);
    pinMode(echo, INPUT);
}

float Ultrasonic::sendPing()
{
    digitalWrite(m_trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(m_trigger, LOW);
    long duration = pulseIn(m_echo, HIGH);
    float distance = duration * 0.034 / 2;
    return distance;
}
