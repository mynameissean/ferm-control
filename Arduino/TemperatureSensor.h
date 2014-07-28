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
class TemperatureSensor
{
private:
    float m_Temperature;
    float m_TemperatureTarget;
    float m_TemperatureBand;
    byte* m_SensorAddress;
    char* m_FriendlyName;
    int m_FriendlyNameLength;

    void DebugPrintSensor(byte*);
    TemperatureSensor();

public:
    
    TemperatureSensor(byte *SensorAddress, float, float, char*, int);
    TempInRange ShouldBeginTemperatureAdjustment();
    bool RetrieveTemperatureFromSensor(OneWire);
    bool DoesSensorExist(OneWire Sensors);
    float GetTemperature(){return m_Temperature;};
    bool HaveHitTemperatureTarget(bool);
    char* GetFriendlyName(){return m_FriendlyName;};
    int GetFriendlyNameLength(){return m_FriendlyNameLength;};
    void TrimNameToLength(int);
    

    
};
#endif

