#ifndef _TEMPERATUREMANAGER_H_
#define _TEMPERATUREMANAGER_H_

#include "OneWire.h"
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include "Relay.h"
#include "TemperatureSensor.h"
#include "SensorLinkedList.h"

class TemperatureManager
{    
    public:
		TemperatureManager(OneWire*);
        TemperatureManager(Relay*, Relay*, TemperatureSensor*, OneWire*);
		void UpdateAllFoundSensors();
    private:
		void FindAllSensors();
		bool AddSensor(byte*, float, float, ID*);

		Relay* m_Heating;
        Relay* m_Cooling;
        TemperatureSensor* m_PrimaryTemp = NULL;
		SensorLinkedList* m_SensorList;
		int m_iNumSensors = 0;
		OneWire* m_OneWireSensor;



};

#endif