// Utility.h

#ifndef _UTILITY_h
#define _UTILITY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class Utility
{
 private:


 public:
	static float ToCelsius(float);
    static float ToFahrenheit(float);
    static unsigned long TimeDifference(unsigned long);
    static bool GetCurrentState(int);
    static void Cycle(int, int, int);
};

//extern Utility UTILITY;

#endif

