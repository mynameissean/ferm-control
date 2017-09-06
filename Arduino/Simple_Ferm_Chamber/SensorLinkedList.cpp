#include "SensorLinkedList.h"

SensorLinkedList::SensorLinkedList()
{
	m_Sensor = NULL;
}

void SensorLinkedList::SetSensor(TemperatureSensor* Sensor)
{
	m_Sensor = Sensor;
}

TemperatureSensor* SensorLinkedList::GetSensor()
{
	return m_Sensor;
}

void SensorLinkedList::Delete()
{
	if (NULL != m_Sensor)
	{
		delete m_Sensor;
		m_Sensor = NULL;
	}
}