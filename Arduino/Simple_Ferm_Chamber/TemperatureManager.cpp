#include "TemperatureManager.h"
#include "Definitions.h"
#include "Logger.h"


TemperatureManager::TemperatureManager(OneWire* inSensor)
{
	m_OneWireSensor = inSensor;
	m_SensorList = NULL;
	m_Heating = NULL;
	m_Cooling = NULL;
	m_PrimaryTemp = NULL;
}

TemperatureManager::TemperatureManager(Relay *Heating, Relay *Cooling, TemperatureSensor *Primary, OneWire* inSensor)
{
    m_Heating = Heating;
    m_Cooling = Cooling;
    m_PrimaryTemp = Primary;
	m_OneWireSensor = inSensor;
	m_SensorList = NULL;
}

void TemperatureManager::UpdateAllFoundSensors()
{
	//Clear out all our found sensors
	if (m_iNumSensors > 0)
	{
		for (int i = 0; i < m_iNumSensors; i++)
		{

		}
	}
}

///<summary> Add a sensor to the given list that we'll be monitoring </summary>
///<param name="SensorAddress">The ID of the sensor we're to add</param>
///<return> True if the addition was unique, false if the ID already existed</return>
bool TemperatureManager::AddSensor(byte* SensorAddress, float Band, float Temp, ID* Id)
{
	bool retVal = true;
	TemperatureSensor* sensor = NULL;
	SensorLinkedList* sensorNode = NULL;

	if (NULL == m_SensorList)
	{
		//Initialize our list of sensors
		m_SensorList = new SensorLinkedList();
	}
	SensorLinkedList* current = m_SensorList;
	do
	{
		if (true == current->GetSensor()->IsEqual(SensorAddress))
		{
			//No need to add them, they already exist
			retVal = false;
			goto cleanup;
		}
		//Go on to the next element
		current = (SensorLinkedList*)m_SensorList->GetNext();
	} while (NULL != current->GetNext());

	//Unique address, add it to our list
	sensor = new TemperatureSensor(SensorAddress, Band, Temp, Id);
	sensorNode = new SensorLinkedList();
	sensorNode->SetSensor(sensor);
	//After traversing through all the nodes, current contains our last element
	//in the linked list
	current->SetNext(sensorNode);
	
	
cleanup:
	return retVal;
}

///<summary>Go through the collection of sensors and print all that we have </summary>
void TemperatureManager::FindAllSensors()
{		
	bool retVal = false;
	byte foundSensorAddress[SENSOR_ADDRESS_LENGTH];
	Logger::Log("Searching for temperature sensors", DEB);		

	
	while (true == m_OneWireSensor->search(foundSensorAddress))
	{
		bool found = true;


		//Print out the data
		Logger::PrependLogStatement(DEB);
		Logger::LogStatement(F("Found sensor "), DEB);
		Logger::LogStatement(foundSensorAddress, DEB);
		Logger::EndLogStatement(DEB);


		//See if this is a valid address
		if (OneWire::crc8(foundSensorAddress, 7) != foundSensorAddress[7]) {
			Logger::Log(F("CRC is not valid!"), WAR);
			m_OneWireSensor->reset();
			m_OneWireSensor->reset_search();
			break;
		}
		if (foundSensorAddress[0] != 0x10 && foundSensorAddress[0] != 0x28) {
			Logger::Log(F("Device is not recognized"), WAR);
			continue;
		}
		
	}

	Logger::Log(F("Done searching for sensors"), DEB);



	cleanup:
		//Reset the temperature sensor search for subsequent invocations
		m_OneWireSensor->reset_search();
}

