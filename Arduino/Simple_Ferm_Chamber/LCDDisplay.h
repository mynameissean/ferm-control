#ifndef _LCDDISPLAY_h
#define _LCDDISPLAY_h
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Logger.h"
#include <Wire.h>
#include <Encoder.h>

class LCDDisplay : public Display
{
    private: 
    public: 
        LCDDisplay();
        


};

#endif