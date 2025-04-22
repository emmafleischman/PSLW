#include <Arduino.h>
#include "Ultrasonic.h"
#include "Wire.h"

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

  // unsigned long beginning = micros();
  // unsigned long startTime = micros();
  // while (digitalRead(m_echo) == LOW) {
  //   startTime = micros();
  //   if(startTime - beginning > 1000000)
  //   {
  //     // Serial.println("PING TIMEOUT");
  //     return -1;
  //   }
  // }

  // unsigned long endTime = micros();
  // while (digitalRead(m_echo) == HIGH) {
  //   endTime = micros();  
  // }
  // unsigned long duration = endTime - startTime;
  long duration = pulseIn(m_echo, HIGH);
  float distance = duration * 0.0343 / 2;
  // float distance = (duration / 29.0) / 2.0;  // Divide by 29 to convert microseconds to cm, divide by 2 for round-trip
  return distance;
}
