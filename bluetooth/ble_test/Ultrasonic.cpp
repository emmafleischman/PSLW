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
  digitalWrite(m_trigger, LOW);
  delayMicroseconds(2);  
  digitalWrite(m_trigger, HIGH);  
  delayMicroseconds(10);  
  digitalWrite(m_trigger, LOW);  

  unsigned long startTime = micros();
  while (digitalRead(m_echo) == LOW) {
    startTime = micros();  
  }

  unsigned long endTime = micros();
  while (digitalRead(m_echo) == HIGH) {
    endTime = micros();  
  }

  // Calculate the duration of the pulse (time between start and end)
  unsigned long duration = endTime - startTime;

  // Calculate the distance in centimeters (speed of sound = 29 microseconds per cm)
  float distance = (duration / 29.0) / 2.0;  // Divide by 29 to convert microseconds to cm, divide by 2 for round-trip

  return distance;
}
