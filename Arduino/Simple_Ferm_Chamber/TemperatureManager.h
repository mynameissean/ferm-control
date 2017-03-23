#ifndef _TEMPERATUREMANAGER_H_
#define _TEMPERATUREMANAGER_H_

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include "Relay.h"
#include "TemperatureSensor.h"

class TemperatureManager
{    
    public:
        TemperatureManager(Relay*, Relay*, TemperatureSensor*);
    private:
        Relay* m_Heating;
        Relay* m_Cooling;
        TemperatureSensor* m_PrimaryTemp;
		TemperatureSensor m_SecondarySensors[];


};

#endif