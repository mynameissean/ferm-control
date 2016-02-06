// Utility functions useful in operation of the system

#include "Utility.h"
#include "Definitions.h"
#include "EEPROMex.h"
#include "Logger.h"


/**
 *Convert the temperature to fahrenheit from celsius.
 *@param Temperature the temperature in degrees celsius
 *@return The temperature in degrees fahrenheit
 */
 float Utility::ToFahrenheit(float Temperature)
 {
   float retVal = INVALID_DATA;
   retVal = Temperature * 9.0 / 5.0 + 32.0; 
   return retVal;
 }
 
 /**
 *Convert the temperature to celsius from fahrenheit.
 *@param Temperature the temperature in degrees fahrenheit
 *@return The temperature in degrees celsius
 */
 float Utility::ToCelsius(float Temperature)
 {
    float retVal = INVALID_DATA;
    retVal =  (Temperature - 32.0) * 5.0 / 9.0;
    return retVal;
 }

 ///<summary>Get the difference between the current time and the passed in 
 ///time.  This function handles overflow as gracefully as possible by assuming
 ///that the passed in time is in the past.</summary>
 ///<param name="PreviousTime">The time difference we want to calculate.  This time is in the past</param>
 ///<return>The difference between the current time and the previous time</return>
 unsigned long Utility::TimeDifference(unsigned long PreviousTime)
 {
     unsigned long retVal = INVALID_TIME;

     //Get the current time
     unsigned long currentTime = millis();
     if(PreviousTime > currentTime)
     {
         //We've rolled over.  Do the math to figure out how much time has passed.
         retVal = UNSIGNED_LONG_MAX_VALUE - PreviousTime;
         retVal += currentTime;
     }
     else {
         //No rollover.  Do the normal calculation
         retVal = currentTime - PreviousTime;
     }

	 Logger::PrependLogStatement(DEB);
     Logger::LogStatement(F("Time difference is "), DEB);
     Logger::LogStatement(retVal, DEB);


     return retVal;
 }

///<summary>Get the status of the current pin.  This will determine whether it is in at a HIGH
///or LOW state.</summary>
///<return>True if high, false if low</return>
bool Utility::GetCurrentState(int Pin)
{
    bool retVal = false;
    bitRead(PORTD, Pin);
    return retVal;
}

///<summary> Cycle the LED pin on and off </summary>
///<param Name="Pin>Output LED pin number </param>
///<param Name="OnTime">How long to keep the LED on for </param>
///<param Name="OffTime">How long to keep the LED off for </param>
void Utility::Cycle(int Pin, int OnTime, int OffTime)
{
  digitalWrite(Pin, HIGH);   // set the LED on
  delay(OnTime);              // wait for a second
  digitalWrite(Pin, LOW);    // set the LED off
  delay(OffTime);              // wait for a second
}

///<summary> Cycle the LED pin on and off a set number of times </summary>
///<param name="Pin">Output LED pin number </param>
///<param name="TimesToFlash"> How many 100ms cycles of on and off to go through</param>
void Utility::Flash(int Pin, int TimesToFlash)
{
	for(int i = 0; i < TimesToFlash; i++)
	{
		Cycle(Pin, 100, 100);
	}
}

///<summary>Update the float at the specified address so it is stored in the EEPROM</summary>
///<param name="Address">Where the float is stored</param>
///<param name="Value">What the value is to store</param>
void Utility::UpdateEEPROMFloat(int Address, float Value)
{
    if(0 == EEPROM.updateFloat(Address, Value))
    {

        Logger::Log(F("Unable to write to eeprom"), WAR);

    }
}

char* Utility::ftoa(char *a, double f, int precision)
{
 long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
 
 char *ret = a;
 long heiltal = (long)f;
 itoa(heiltal, a, 10);
 while (*a != '\0') a++;
 *a++ = '.';
 long desimal = abs((long)((f - heiltal) * p[precision]));
 itoa(desimal, a, 10);
 return ret;
}


 //Utility UTILITY;
