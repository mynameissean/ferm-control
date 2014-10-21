// Relay.h

#ifndef _RELAY_h
#define _RELAY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Triggerable.h";

class Relay : public Triggerable
{
 private:
     int m_TriggerPin;
     int m_DisplayPin;
     unsigned long m_CompressorDelay;
     unsigned long m_MinRunTime;
     unsigned long m_OffTimeStart;
     unsigned long m_OnTimeStart;
     bool m_IsOn;
     bool CanTurnOn();
     bool CanTurnOff();

 public:
	Relay(int TriggerPin, int DisplayPin);
    Relay(int TriggerPin, int DisplayPin, unsigned long MinRunTime, unsigned long CompressorDelay);
    bool TurnOn();
    bool TurnOff();
    bool IsOn(){return m_IsOn;}
    int GetDisplayPin(){return m_DisplayPin;};
};
#endif


