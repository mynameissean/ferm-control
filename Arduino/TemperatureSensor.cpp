
#include "TemperatureSensor.h"
#include "Utility.h"



 ///<summary>Generate and encapsulate all the data related to a temperature sensor.</summary>
 ///<param name="SensorAddress">The GUID for the sensor on the system</param>
 ///<param name="TempBand">The target range the temperature can be</param>
 ///<param name="TempTarget">The target temperature for the sensor</param>
TemperatureSensor::TemperatureSensor(byte* SensorAddress, float TempBand, float TempTarget, char* FriendlyName, int FriendlyNameLength)
{
    m_SensorAddress = SensorAddress;
    m_TemperatureBand = TempBand;
    m_TemperatureTarget = TempTarget;
    m_FriendlyName = FriendlyName;
    m_FriendlyNameLength = FriendlyNameLength;
}

///<summary>Determine whether our temperature is in our range of control. </summary>
///<return>The enumeration value representing what state we're in</return>
 TempInRange TemperatureSensor::ShouldBeginTemperatureAdjustment()
 {
    TempInRange retVal;    
   if(m_Temperature > m_TemperatureTarget + m_TemperatureBand )
    {
       //We need to adjust our temperature, it's too high
        retVal = TOO_HOT;
    }
   else if(m_Temperature < m_TemperatureTarget - m_TemperatureBand)
   {
	   //We need to adjust our temperature, it's too low
       retVal = TOO_COLD;
   }
   else
   {
       retVal = JUST_RIGHT;
   }
   
   return retVal;
 }

 ///<summary>This tells us whether we've hit the target temperature within the allowable
 ///range.  If it's set to 65 with 1 degree of range, then we'll return true once we hit
 ///between 64 and 66.  This is usually a much smaller range than the temperature band
 ///we'll begin cooling at.  </summary.>
 ///<param name="Cooling"> True if the temperature is falling, false otherwise</param>
 ///<return>True if we've hit the temperature, false otherwise</return>
 bool TemperatureSensor::HaveHitTemperatureTarget(bool Cooling)
 {
     bool retVal = false;
     if(true == Cooling)
     {
         //Temperature is falling.  See if we've gone past it
         if(m_Temperature <= m_TemperatureTarget)
         {
             retVal = true;
         }
     }
     else
     {
         //Temperature is rising.  See if we've gone past it
         if(m_Temperature >= m_TemperatureTarget)
         {
             retVal = true;
         }
     }

     return retVal;
 }

 ///<summary> Retrieve the temperature from the OneWire sensor</summary>
 ///<param name="Sensors">Location of the bus for the OneWire sensors</param>
 ///<return>True if successful, false otherwise</return>
 bool TemperatureSensor::RetrieveTemperatureFromSensor(OneWire Sensors)
 {
     float retVal = INVALID_DATA;
     byte data[2];
   //Found the sensor we were looking for, read the data from it
     if(0 == Sensors.reset())
     {
       Serial.println("Unable to reset the bus");
       goto cleanup;
     }  
     
     Sensors.select(m_SensorAddress);
     
     //Send the 0x44 command, which is the convert command
     Sensors.write(0x44);
     if(0 == Sensors.reset())
     {
       Serial.println("Unable to reset the bus after converting temperature");
       goto cleanup;
     }
     Sensors.select(m_SensorAddress);
     
     //Send the 0xBE command, which is read the scratchpad
     Sensors.write(0xBE);     
     
     //The data is of the format LSB, MSB
     data[0] = Sensors.read();
     data[1] = Sensors.read();
     retVal = ((data[1] << 8) | data[0]); //using two's compliment
     retVal = retVal / 16.0;
     retVal = Utility::ToFahrenheit(retVal);
     
     //Reset the communication
     Sensors.reset();
     
     //Assign our temperature data
     m_Temperature = retVal;
cleanup:     
     return retVal;
 }


///<summary>Go through the collection of sensors and validate that we have the
///requested sensor on the system.</summary>
///<param name="Sensors"> The OneWire bus that contains the temperature sensors</param>
///<return> True if found, false otherwise</return>
 bool TemperatureSensor::DoesSensorExist(OneWire Sensors)
 {
   bool retVal = false;
   byte foundSensorAddress[8];

#ifdef _DEBUG
   Serial.print("Searching for ");
   DebugPrintSensor(m_SensorAddress);
#endif 

   while(true == Sensors.search(foundSensorAddress))
   {
     bool found = true;   
     
#ifdef _DEBUG
     //Print out the data
     Serial.println("Found Sensor: ");
      DebugPrintSensor(foundSensorAddress);     
#endif 

     //See if this is a valid address
     if ( OneWire::crc8( foundSensorAddress, 7) != foundSensorAddress[7]) {
          Serial.println("CRC is not valid!");
          Sensors.reset();
          Sensors.reset_search();
          break;
      }     
      if ( foundSensorAddress[0] != 0x10 && foundSensorAddress[0] != 0x28) {
        Serial.println("Device is not recognized");
        continue;
      }
      
     //We found a valid device on the wire.  See if it's a match
     for(int i = 0; i < 8; i++)
     {
       if(m_SensorAddress[i] != foundSensorAddress[i])
       {
         found = false;
         break;
       }
     }
     //See if we found a match
     if(true == found)
     {
       //This is the sensor we're looking for
#ifdef _DEBUG
       Serial.println("Found sensor");
#endif
       retVal = true;
       break;      
     }     
   }  
   Serial.println("Done searching for sensors");
   
cleanup:
  //Reset the temperature sensor search for subsequent invocations
  Sensors.reset_search();
  return retVal;   
 }
 
 ///<summary>Trim off the trailing part of the friendly name.  This puts the null terminator earlier in the string</summary>
 ///<param name="Length">How long the name should be</param>
 void TemperatureSensor::TrimNameToLength(int Length)
 {
     m_FriendlyNameLength = Length;

     m_FriendlyName[Length - 1] = '\0';
 }

  
 /**
  * Print out the sensor data in a readable format.
  */
  void TemperatureSensor::DebugPrintSensor(byte* SensorAddress)
 {
    Serial.print("Device");
      for(int i = 0; i < 8; i++)
      {
        Serial.print(SensorAddress[i]);
        Serial.print("-");        
      }
      Serial.println("");
 }