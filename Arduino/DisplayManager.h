#ifndef _DISPLAYMANAGER_h
#define _DISPLAYMANAGER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "I2CDisplay.h"
#include "TemperatureSensor.h"

class DisplayManager
{
private:
    I2CDisplay* m_Display;
public:
    enum DisplayTypes {
        LCD2041 = 0
    } ;

    DisplayManager(int, DisplayTypes);
    void ShowTemperature(TemperatureSensor**, int);
    void ShowString(char*, int, bool, bool);
};
#endif