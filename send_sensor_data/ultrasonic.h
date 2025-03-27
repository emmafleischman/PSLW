#ifndef _ULTRASONIC_H_
#define _ULTRASONIC_H_

typedef enum {
    FRONT, 
    BACK
} SensorPosition;

class Ultrasonic
{
    public:
        Ultrasonic();
        Ultrasonic(int frontTrig, int frontEcho, int backTrig, int backEcho);
        float sendPing(SensorPosition pos);
    private:
        int m_front_trig;
        int m_front_echo;
        int m_back_trig;
        int m_back_echo;
};

#endif
