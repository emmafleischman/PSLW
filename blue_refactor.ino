#include <bluefruit.h>
#include <Wire.h>

#include "Adafruit_DRV2605.h"
#include <Adafruit_Sensor.h>
#include "Ultrasonic.h"
#include "StepLength.h"

#define DEBUG

#define TRIGGER   6
#define ECHO      10

#define START_SESSION   2
#define STOP_SESSION    3
#define PAUSE_SESSION   4

enum State {
    INIT,
    ADVERTISING,
    CONNECTING,
    WAIT_FOR_START,
    WAIT_FOR_RESUME,
    CALIBRATION,
    ALGORITHM
};
State current_state = INIT;

Adafruit_DRV2605 buzzer;

BLEService stepLengthService("11111111-1111-1111-1111-111111111111");
BLECharacteristic stepLengthChar("22222222-2222-2222-2222-222222222222");
BLECharacteristic sessionChar("33333333-3333-3333-3333-333333333333");
bool connected = 0;
float stepLength = 0.0;

bool session_start = 0;
bool session_stop = 0;
bool session_pause = 0;

Ultrasonic front(TRIGGER, ECHO);

void setup()
{
    #ifdef DEBUG
        Serial.begin(9600);
        while(!Serial) delay(10);
    #endif
    pinMode(LED_BUILTIN,  OUTPUT);

    buzzer.begin();
    buzzer.setMode(DRV2605_MODE_INTTRIG); // default, internal trigger when sending GO command
    buzzer.selectLibrary(1);
    buzzer.setWaveform(0, 47);  
    buzzer.setWaveform(1, 0);  

    // Initialize BLE
    Bluefruit.begin();
    Bluefruit.setName("StepLengthSensor");

    Bluefruit.Periph.setConnectCallback(connect_callback);
    Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

    // Start the service first
    stepLengthService.begin();

    // Configure the float characteristic for read/write and notify access
    stepLengthChar.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);  // Read and Notify
    stepLengthChar.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);  // No write access, only read/notify
    stepLengthChar.setFixedLen(sizeof(float));  // 4 bytes for float
    stepLengthChar.begin();

    sessionChar.setProperties(CHR_PROPS_WRITE);
    sessionChar.setPermission(SECMODE_OPEN, SECMODE_OPEN); // write allowed
    sessionChar.setFixedLen(sizeof(float));
    sessionChar.setWriteCallback(write_callback);  // Callback when central writes
    sessionChar.begin();

    // Set the initial value for the characteristic
    stepLengthChar.write((uint8_t *)&stepLength, sizeof(float));

    Bluefruit.Advertising.addService(stepLengthService);
    Bluefruit.Advertising.addName();

    current_state = ADVERTISING;
}

void loop()
{
    switch(current_state)
    {
        case ADVERTISING:
        {
            // Start advertising
            Bluefruit.Advertising.start(10);
            #ifdef DEBUG
                Serial.println("BLE Advertising.");
            #endif
            current_state = CONNECTING;
            break;
        }
        case CONNECTING:
        {
            #ifdef DEBUG
                Serial.println("Waiting to connect...");
            #endif
            uint64_t start = millis();
            while(millis() - start < 10000)
            {
                if(connected)
                {
                    current_state = WAIT_FOR_START;
                    return;
                }
            }
            current_state = CALIBRATION;
            break;
        }
        case WAIT_FOR_START:
        {
            #ifdef DEBUG
                Serial.println("Waiting for start session...");
            #endif
            if(session_start)
            {
                session_start = 0;
                current_state = CALIBRATION;
            }
            break;
        }
        case WAIT_FOR_RESUME:
        {
            #ifdef DEBUG
                Serial.println("Waiting to resume session...");
            #endif
            if(session_start)
            {
                session_start = 0;
                current_state = ALGORITHM;
            }
            else if(session_stop)
            {
                session_stop = 0;
                current_state = WAIT_FOR_START;
            }
            break;
        }
        case CALIBRATION:
        {
            #ifdef DEBUG
                Serial.println("Calibration Beginning.");
            #endif

            // BUZZ 3x
            for(int i=0; i<3; i++)
            {
                buzzer.go();
                delay(400);
            }

            stepLength = runCalibration(&front);

            if(connected)
            {
                while(!stepLengthChar.notifyEnabled()){};
                stepLengthChar.notify((uint8_t *)&stepLength, sizeof(float));
                #ifdef DEBUG
                    Serial.print("Sent target step length: ");
                    Serial.println(stepLength, 2);
                #endif
            }

            for(int i=0; i<3; i++)
            {
                buzzer.go();
                delay(400);
            }

            #ifdef DEBUG
                Serial.print("Calibration Complete. Target Step Length: ");
                Serial.println(target_step_length);
            #endif

            current_state = ALGORITHM;
            break;
        }
        case ALGORITHM:
        {
            runAlgorithm(&front, &buzzer, &stepLengthChar, &session_pause);
            session_pause = 0;
            current_state = WAIT_FOR_RESUME;
            break;
        }
    }
}

void connect_callback(uint16_t conn_handle) {
    #ifdef DEBUG
    Serial.println("Central connected.");
    #endif 
    connected = 1;
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  #ifdef DEBUG
  Serial.println("Central disconnected.");
  #endif
  connected = 0;
}

void write_callback(uint16_t conn_handle, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
    float receivedValue;
    memcpy(&receivedValue, data, sizeof(float));
    if(receivedValue == START_SESSION)
    {
        session_start = 1;
        #ifdef DEBUG
        Serial.println("Session Started.");
        #endif
    }
    else if(receivedValue == PAUSE_SESSION)
    {
        session_pause = 1;
        #ifdef DEBUG
        Serial.println("Session Paused.");
        #endif
    }
    else if(receivedValue == STOP_SESSION)
    {
        session_stop = 1;
        #ifdef DEBUG
        Serial.println("Session Paused.");
        #endif
    }
}