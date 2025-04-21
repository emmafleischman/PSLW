#ifndef _BUZZER_H_
#define _BUZZER_H_
#include "Adafruit_DRV2605.h"

class Buzzer
{
    public:
        Buzzer();
        void tripleBuzz();
        void singleBuzz();
    private:
        Adafruit_DRV2605 m_buzzer;
};

#endif
