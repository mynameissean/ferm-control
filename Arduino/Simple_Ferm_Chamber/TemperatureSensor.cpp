
#include "TemperatureSensor.h"
#include "Utility.h"
#include "Logger.h"


 ///<summary>Generate and encapsulate all the data related to a temperature sensor.</summary>
 ///<param name="SensorAddress">The GUID for the sensor on the system</param>
 ///<param name="TempBand">The target range the temperature can be</param>
 ///<param name="TempTarget">The target temperature for the sensor</param>
TemperatureSensor::TemperatureSensor(byte* SensorAddress, float TempBand, float TempTarget, ID* InId)
{
    m_SensorAddress = SensorAddress;
    m_TemperatureBand = TempBand;
    m_TemperatureTarget = TempTarget;
    m_ID = InId;
}

///<summary>Update the target temperature for this sensor</summary>
///<param name="TempTarget">The new temperature value to shoot for</param>
void TemperatureSensor::SetTargetTemperature(float TempTarget)
{
    m_TemperatureTarget = TempTarget;
}

///<summary>Set the target temperature band</summary>
///<param name="TempBand">How wide the temperature is allowed to swing and still be in range</param>
void TemperatureSensor::SetTemperatureBand(float TempBand)
{
    m_TemperatureBand = TempBand;
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
       Logger::Log(F("Unable to reset the bus"), ERR);
       goto cleanup;
     }  
     
     Sensors.select(m_SensorAddress);
     
     //Send the 0x44 command, which is the convert command
     Sensors.write(0x44);

	 //TODO: Can only do this when not in parasitic power mode
	 while(Sensors.read() == 0) 
	 {
		 //Chill for 5 ms to see if it's all done
		 delay(5);
	 }

	 //TODO: This might not be needed for non-parasite powered sensors
     if(0 == Sensors.reset())
     {
       Logger::Log(F("Unable to reset the bus after converting temperature"), ERR);
       goto cleanup;
     }
     Sensors.select(m_SensorAddress);
	 
	 

     //Send the 0xBE command, which is read the scratchpad
     Sensors.write(0xBE);     
     
     //The data is of the format LSB, MSB
     data[0] = Sensors.read();
     data[1] = Sensors.read();
	 retVal = ((data[1] << SENSOR_ADDRESS_LENGTH) | data[0]); //using two's compliment
     retVal = retVal / 16.0;
     retVal = Utility::ToFahrenheit(retVal);
     
     //Reset the communication
     Sensors.reset();
     
     //Assign our temperature data
     m_Temperature = retVal;
cleanup:     
     return retVal;
 }

 ///<summary>Print out the temperature sensor in the following format:
 ///SensorName:Temperature</summary>
 void TemperatureSensor::Print()
 {
	 char buffer[15];
     char* buf = Utility::ftoa(buffer, m_Temperature, 2);
	 Logger::LogCommunicationStatement(m_ID->GetName(), buf);	 
 }


///<summary>Go through the collection of sensors and validate that we have the
///requested sensor on the system.</summary>
///<param name="Sensors"> The OneWire bus that contains the temperature sensors</param>
///<return> True if found, false otherwise</return>
 bool TemperatureSensor::DoesSensorExist(OneWire Sensors)
 {
   bool retVal = false;
   byte foundSensorAddress[SENSOR_ADDRESS_LENGTH];

   Logger::PrependLogStatement(DEB);
   Logger::LogStatement(F("Searching for "), DEB);   
   Logger::LogStatement(m_SensorAddress, DEB);
   Logger::EndLogStatement(DEB);

   while(true == Sensors.search(foundSensorAddress))
   {
     bool found = true;   
     

     //Print out the data
	 Logger::PrependLogStatement(DEB);
     Logger::LogStatement(F("Found sensor "), DEB);   
	 Logger::LogStatement(foundSensorAddress, DEB);
	 Logger::EndLogStatement(DEB);        


     //See if this is a valid address
     if ( OneWire::crc8( foundSensorAddress, 7) != foundSensorAddress[7]) {
          Logger::Log(F("CRC is not valid!"), WAR);
          Sensors.reset();
          Sensors.reset_search();
          break;
      }     
      if ( foundSensorAddress[0] != 0x10 && foundSensorAddress[0] != 0x28) {
        Logger::Log(F("Device is not recognized"), WAR);
        continue;
      }
      
     //We found a valid device on the wire.  See if it's a match
	  for (int i = 0; i < SENSOR_ADDRESS_LENGTH; i++)
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
       Logger::Log(F("Found sensor"), DEB);
       retVal = true;
       break;      
     }     
   }  

   Logger::Log(F("Done searching for sensors"), DEB);


   
cleanup:
  //Reset the temperature sensor search for subsequent invocations
  Sensors.reset_search();
  return retVal;   
 }

  ///<summary>Print out the sensor data in a readable format.</summary>
  ///<param name="SensorAddress">The address of the sensor to print out</param>
  void TemperatureSensor::DebugPrintSensor(byte* SensorAddress)
 {

	Logger::PrependLogStatement(DEB);
	Logger::LogStatement(F("Device "), DEB);
	for (int i = 0; i < SENSOR_ADDRESS_LENGTH; i++){
		Logger::LogStatement(SensorAddress[i], DEB);
        Logger::LogStatement(F("-"), DEB);        
    }
	Logger::EndLogStatement(DEB);
 }

  int TemperatureSensor::foo()
  {
	  return 1;
  }

  ///<summary>Determine whether or not this Sensor is equivalent to another</summary>
  ///<param name="SensorAddress">The sensor address to compare to</summary>
  ///<return> True if they're the same, false otherwise </summary>
  bool TemperatureSensor::IsEqual(byte* SensorAddress)
  {
	  bool retVal = true;

	  //Go through each byte and see if we find a difference
	  for (int i = 0; i < SENSOR_ADDRESS_LENGTH; i++)
	  {
		  if (SensorAddress[i] != m_SensorAddress[i])
		  {
			  //Don't match
			  retVal == false;
			  break;
		  }
	  }

	  return retVal;
  }

