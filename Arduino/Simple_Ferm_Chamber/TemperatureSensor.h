// TemperatureSensor.h

#ifndef _TEMPERATURESENSOR_h
#define _TEMPERATURESENSOR_h

#include "OneWire.h"
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Definitions.h"
#include "ID.h"

class TemperatureSensor
{
private:
    float m_Temperature;
    float m_TemperatureTarget;
    float m_TemperatureBand;
    byte* m_SensorAddress;
    ID *m_ID;

    void DebugPrintSensor(byte*);
    TemperatureSensor();

public:
    
    TemperatureSensor(byte *SensorAddress, float, float, ID*);
    TempInRange ShouldBeginTemperatureAdjustment();
    bool RetrieveTemperatureFromSensor(OneWire);
    bool DoesSensorExist(OneWire Sensors);
    float GetTemperature(){return m_Temperature;};
    bool HaveHitTemperatureTarget(bool);
    ID* GetID(){return m_ID;};
    void TrimNameToLength(int);
    void SetTargetTemperature(float);
    void SetTemperatureBand(float);
    

    
};
#endif

