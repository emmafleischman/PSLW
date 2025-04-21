#include "Buzzer.h"

Buzzer::Buzzer() 
{
    m_buzzer.begin();
    m_buzzer.setMode(DRV2605_MODE_INTTRIG); // default, internal trigger when sending GO command
    m_buzzer.selectLibrary(1);
}

void Buzzer::tripleBuzz()
{
    m_buzzer.setWaveform(0, 1);  // ramp up medium 1, see datasheet part 11.2
    m_buzzer.setWaveform(1, 0);  // strong click 100%, see datasheet part 11.2
    m_buzzer.setWaveform(0, 1);  // ramp up medium 1, see datasheet part 11.2
    m_buzzer.setWaveform(1, 0);  // strong click 100%, see datasheet part 11.2
    m_buzzer.setWaveform(0, 1);  // ramp up medium 1, see datasheet part 11.2
    m_buzzer.setWaveform(1, 0);  // strong click 100%, see datasheet part 11.2
    m_buzzer.go();
}

void Buzzer::singleBuzz()
{
    m_buzzer.setWaveform(0, 1);  // ramp up medium 1, see datasheet part 11.2
    m_buzzer.setWaveform(1, 0);  // strong click 100%, see datasheet part 11.2
    m_buzzer.go();
}