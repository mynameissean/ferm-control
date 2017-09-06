#ifndef _SENSORLINKEDLIST_H_
#define _SENSORLINKEDLIST_H_


#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "LinkedList.h"
#include "TemperatureSensor.h"

class SensorLinkedList : public LinkedList
{
public:
	SensorLinkedList();
	void SetSensor(TemperatureSensor*);
	TemperatureSensor* GetSensor();
	void Delete();

private:
	LinkedList* m_Next;
	TemperatureSensor* m_Sensor;
};

#endif