#ifndef _ULTRASONIC_H_
#define _ULTRASONIC_H_

class Ultrasonic
{
    public:
        Ultrasonic();
        Ultrasonic(int trigger, int echo);
        float sendPing();
    private:
        int m_trigger;
        int m_echo;
};

#endif