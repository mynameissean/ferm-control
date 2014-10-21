#ifndef _TRIGGERABLE_h
#define _TRIGGERABLE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif


class Triggerable
{    	
private:
    int m_TriggerPin;
    int m_DisplayPin;
    void Trigger(int);

public:
    Triggerable(int);
    Triggerable(int, int);
    void TriggerHigh();
    void TriggerLow();

};

#endif