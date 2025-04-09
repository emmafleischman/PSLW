#include <ArduinoBLE.h>
#include "Ultrasonic.h"

#define FRONT_TRIGGER   4
#define FRONT_ECHO      3
#define BACK_TRIGGER    6
#define BACK_ECHO       5

Ultrasonic front(FRONT_TRIGGER, FRONT_ECHO);
Ultrasonic back(BACK_TRIGGER, BACK_ECHO);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // turn on the built-in LED
    
    Serial.begin(115200);
    while(!Serial){}
    digitalWrite(LED_BUILTIN, LOW);
}

#define BURST_SIZE    10
#define MAX_LEN       50
#define CHECK_COUNT   3
#define WINDOW_SIZE   5
#define POLL_DELAY    20
#define SWITCH_DELAY  0

int idx = 0;
float burst[BURST_SIZE];
bool poll_front = true;

float old_steps[WINDOW_SIZE];
float recent_steps[WINDOW_SIZE];

void print_burst()
{
  for(int i=0; i<BURST_SIZE; i++)
  {
    Serial.print(burst[i]);
    Serial.print(" ");
  }
  Serial.println(".");
}

float average_burst()
{
  float average = 0;
  float n = 0;
  for(int i=0; i<BURST_SIZE; i++)
  {
    if(burst[i] < MAX_LEN)
    {
      average += burst[i];
      n++;
    }
  }
  return average / n;
}

void loop() {
  if(poll_front) // TODO: condense this into a function or something so its not so long
  {
    // Serial.println("FRONT");
    while(idx < CHECK_COUNT)
    {
      float frontData = front.sendPing();
      if(frontData < MAX_LEN)
      {
        burst[idx] = frontData;
        idx++;
      }
      else
      {
        idx = 0;
      }
      delay(POLL_DELAY);
    }
    idx = 0;
    for(int i=3; i<BURST_SIZE; i++)
    {
      float data = front.sendPing();
      burst[i] = data;
      delay(POLL_DELAY);
    }
    // Serial.print("Burst: ");
    // print_burst();
    float step_len = average_burst();
    Serial.print("FRONT STEP: ");
    Serial.println(step_len);
    poll_front = false;
    delay(SWITCH_DELAY);
  }
  else
  {
    // Serial.println("BACK");
    while(idx < CHECK_COUNT)
    {
      float backData = back.sendPing();
      if(backData < MAX_LEN)
      {
        burst[idx] = backData;
        idx++;
      }
      else
      {
        idx = 0;
      }
      delay(POLL_DELAY);
    }
    idx = 0;
    for(int i=3; i<BURST_SIZE; i++)
    {
      float data = back.sendPing();
      burst[i] = data;
      delay(POLL_DELAY);
    }
    // Serial.print("Burst: ");
    // print_burst();
    float step_len = average_burst();
//    Serial.print("BACK  STEP: ");
//    Serial.println(step_len);
    poll_front = true;
  }
//  poll_front = ~poll_front;
}
     
  
