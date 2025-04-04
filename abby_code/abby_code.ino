#define purpleTrig    4
#define purpleEcho    3

#include "Ultrasonic.h"

Ultrasonic front(4,3);

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    float distance = front.sendPing();
    
    Serial.print(" Distance: ");
    Serial.println(distance);
    
    delay(2000);

}
