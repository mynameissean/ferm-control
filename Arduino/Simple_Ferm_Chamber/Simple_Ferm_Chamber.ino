#include <Wire.h>
#include "Relay.h"
#include "Utility.h"
#include "TemperatureSensor.h"
#include "Definitions.h"
#include <OneWire.h>
#include "Communicator.h"
#include "EEPROMex.h"
#include "Logger.h"
/*
 * This is a stripped down version of the fermentation controller
 * project.  It doesn't allow for command and control via
 * external software program.  It only sends output out to the 
 * wire for monitoring purposes.
 */
#undef _DEBUG
//#define _DEBUG
int TempReadPin = 3;
int CoolingRelayPin = 4;
int CoolingDisplayPin = 5;
int HeatingDisplayPin = 6;
int HeatingRelayPin = 7;
int StatusLED = 13;
#define DEBOUNCE_VALUE 5
#define CYCLE_TIME 5000
#define DEFAULT_DEBUG_LEVEL ERR

//Global objects
OneWire g_TempSensors(TempReadPin);
byte g_InternalFermentorSensorAddress[8] = {40,118,221,231,3,0,0,173};
byte g_ExternalFermentorAddress[8] = {40,126,182,231,3,0,0,109};
byte g_ambientExternalSensorAddress[8] = {40,118,221,231,3,0,0,173};
byte g_ambientInternalSensorAddress[8] = {40,126,182,231,3,0,0,109};
TemperatureSensor* g_InternalFermentorSensor;
TemperatureSensor* g_ExternalFermentorSensor;
TemperatureSensor* g_AmbientInternalSensor;
Relay* g_Cooling;
Relay* g_Heating;
TempInRange g_LastReading = JUST_RIGHT;
int g_DebounceCounter = 0;
Communicator* m_Communicator = NULL;

//Setup values
float g_PrimaryTemperatureBand = 1.3;
float g_PrimaryTargetTemperature = 83;
unsigned long g_CompressorRunTime = 30000; //30 Seconds
unsigned long g_CompressorOffTime = 240000; //4 Minutes
unsigned long g_HeatingOffTime = 300000;    //5 Minutes

void setup(){
   Serial.begin(9600);
   
   //Set our logging level
   Logger::SetLoggingLevel(DEFAULT_DEBUG_LEVEL);

   pinMode(StatusLED, OUTPUT); 
   pinMode(CoolingDisplayPin, OUTPUT);
   pinMode(HeatingDisplayPin, OUTPUT);
   pinMode(CoolingRelayPin, OUTPUT);
   pinMode(HeatingRelayPin, OUTPUT);



   //Setup our temperature sensors   
   g_InternalFermentorSensor = new TemperatureSensor(g_InternalFermentorSensorAddress, 
                                           g_PrimaryTemperatureBand, 
                                           g_PrimaryTargetTemperature, 
                                           new ID("Internal", strlen("Internal"), 1, 0));
   g_ExternalFermentorSensor = new TemperatureSensor(g_ExternalFermentorAddress, 
                                                   g_PrimaryTemperatureBand, 
                                                   g_PrimaryTargetTemperature, 
                                                   new ID("External_Fermentor", strlen("External_Fermentor"), 2, sizeof(float) * 2));
   g_AmbientInternalSensor = new TemperatureSensor(g_ambientInternalSensorAddress, 
                                                   g_PrimaryTemperatureBand, 
                                                   g_PrimaryTargetTemperature, 
                                                   new ID("Ambient", strlen("Ambient"), 3, sizeof(float) * 4));

   //Setup our relays
   g_Cooling = new Relay(CoolingRelayPin, CoolingDisplayPin, new ID("Cooling", strlen("Cooling"), 1), g_CompressorRunTime, g_CompressorOffTime); //4 minutes
   g_Heating = new Relay(HeatingRelayPin, HeatingDisplayPin, new ID("Heating", strlen("Heating"), 2), 0, g_HeatingOffTime);

   //Setup our controller (if applicable)
   m_Communicator = new Communicator();

   //Signal that we're powered on and ready
   Utility::Cycle(g_Cooling->GetDisplayPin(), 1000, 1000);
   Utility::Cycle(g_Heating->GetDisplayPin(), 1000, 1000);
 }

float ReadFloatFromMemory()
{

}
 
 void loop()
 {  
  //Step 1: See if we have any commands from our overlord
  ReceiveOperatingInstructions();

  //Step 2: Gather our temperature readings     
  bool gathered = GatherTemperatureData();
  if(false == gathered)
  {
      //Can't perform any actions without valid temperature settings.  Turn everything off once we've hit the debounce limit      
      if(true == DebounceTemperatureReading(NO_READING))
      {
          //Shut it all down         
          g_Heating->TurnOff();
          g_Cooling->TurnOff();          
          Utility::Flash(g_Heating->GetDisplayPin(), 10);
          Utility::Flash(g_Heating->GetDisplayPin(), 10);
      }
      goto cleanup;
  }

  //Step 3: Compare them to our values and adjust
  AdjustPrimaryTemperature();

  //Step 4: See if we need to store our current temperature data
  SaveTemperatureData();
  
cleanup:
  //Signal that we're done with this cycle
  Utility::Cycle(StatusLED, 1000, 1000);
  delay(CYCLE_TIME);
 }

 ///<summary>Print out the data so it can be ready by our calling functions.  
 /// Data is written as the following: 
 /// Internal:Temp 
 /// External_Fermentor:Temp
 /// Cooling:On|Off
 /// Heating:On|Off</summary>
 void SaveTemperatureData()
 {
	 g_InternalFermentorSensor->Print();
	 g_ExternalFermentorSensor->Print();
	 g_Cooling->Print();
	 g_Heating->Print();
 }

 
 ///<summary>Open up communication with our controller and see if there are any new commands to receive</summary> 
 void ReceiveOperatingInstructions()
 {
     int ivalue = 0;
     float fvalue = 0.0;
     OperatingCommand systemCommand = INVALID;
     String* command = m_Communicator->Read();
     if(NULL == command || 0 == command->length())
     {
         //Nothing to parse
         goto cleanup;
     }

     //We have a command
     systemCommand = m_Communicator->ParseCommand(command, &ivalue, &fvalue);
     switch(systemCommand)
     {
     case(UTT):
         //Update temperature target (UTT), followed by a XXX.X length float
         if(0 != fvalue)
         {
             g_InternalFermentorSensor->SetTargetTemperature(fvalue);
             //Save stored temperature
             Utility::UpdateEEPROMFloat(g_InternalFermentorSensor->GetID()->GetEEPROMAddress(), fvalue);
         }
         else
         {
			 Logger::Log(F("Unable to update temperature as it's set to 0"), ERR);
         }
         break;
     case(UTB):
         if(fvalue < .1)
         {
             g_InternalFermentorSensor->SetTemperatureBand(fvalue);
             //Save stored band
             Utility::UpdateEEPROMFloat(g_InternalFermentorSensor->GetID()->GetEEPROMAddress() * sizeof(float), fvalue);
         }
         else
         {
             Logger::Log(F("Unable to update temperature band as it's set to less than .1"), ERR);
         }
         break;
     case(RCT):
         if(ivalue != 0)
         {

         }
         else {
			 Logger::Log(F("Cannot report temperature for invalid index"), ERR);
         }
         break;
     case(RRS):
         break;
     case(URS):
         break;
     case(RSI):
         break;
     case(RRI):
         break;
     case(HBS):
         break;
     case(INVALID):
		 Logger::Log(F("Can't operate on invalid command"), ERR);
         break;
     

     }
cleanup:
     return;
 }

 /**
  * Gather the temperature data from the thermistors in the system
  */
 bool GatherTemperatureData()
 {
   bool retVal = false;
   //Go through our sensors     
   if(false == g_InternalFermentorSensor->DoesSensorExist(g_TempSensors))
   {
     Logger::Log(F("Unable to find primary sensor."), ERR);
     goto cleanup;
   }
   //Found the sensor we wanted, get the temperature data   
   
   if(INVALID_DATA == g_InternalFermentorSensor->RetrieveTemperatureFromSensor(g_TempSensors))
   {  
     Logger::Log(F("Unable to get temperature data for primary sensor."), ERR);
     goto cleanup;
   }

   //Got the one we need.  The rest are secondary
   retVal = true;
   if(true == g_ExternalFermentorSensor->DoesSensorExist(g_TempSensors))
   {
     //Get the temperature
     g_ExternalFermentorSensor->RetrieveTemperatureFromSensor(g_TempSensors);
   }
   
   if(true == g_AmbientInternalSensor->DoesSensorExist(g_TempSensors))
   {
     //Get the temperature
     g_AmbientInternalSensor->RetrieveTemperatureFromSensor(g_TempSensors);
   }
     

   //Print out the temperature 
   //Primary
   Logger::PrependLogStatement(DEB);
   Logger::LogStatement(F("Primary Temperature: "), DEB);
   Logger::LogStatement(g_InternalFermentorSensor->GetTemperature(), DEB);
   Logger::EndLogStatement(DEB);
   Logger::PrependLogStatement(DEB);

   //Ambient External
   Logger::LogStatement(F("Ambient External Temperature: "), DEB);
   Logger::LogStatement(g_ExternalFermentorSensor->GetTemperature(), DEB);
   Logger::EndLogStatement(DEB);
   
   //Ambient Internal
   Logger::PrependLogStatement(DEB);
   Logger::LogStatement(F("Ambient Internal Temperature: "), DEB);
   Logger::LogStatement(g_AmbientInternalSensor->GetTemperature(), DEB);
   Logger::EndLogStatement(DEB);

   
cleanup:
   return retVal;
 }
 
 /**
  * Determine if any of our temperature sensors is outside of the necessary ranges.
  */
 void AdjustPrimaryTemperature()
 {
   //We care most about the primary temperature.  This is the temperature we want to control
   TempInRange adjustment = g_InternalFermentorSensor->ShouldBeginTemperatureAdjustment();
   
   if(TOO_HOT == adjustment)
   {
	   Logger::Log(F("Too hot"), DEB);

       if(false == DebounceTemperatureReading(TOO_HOT))
       {
           goto cleanup;
       }
       //Turn the heater off if it's on
       if(false == g_Heating->TurnOff())
       {          
		   Logger::Log(F("Unable to turn the heater off"), WAR);          
       }
       //We need to see if we can activate the cooling
       if(false == g_Cooling->TurnOn())
       {          
            //Can't turn the compressor on yet
		   Logger::Log(F("Can't turn the compressor on"), WAR);
       }
   }
   else if(TOO_COLD == adjustment)
   {
       
        Logger::Log(F("Too cold"), DEB);       
        if(false == DebounceTemperatureReading(TOO_COLD))
        {
            goto cleanup;
        }
       //See if we can turn of the cooling
        if(false == g_Cooling->TurnOff())
       {          
           //Can't turn the compressor on yet
		   Logger::Log(F("Can't turn the compressor off"), WAR);
        }
       //Turn the heater off if it's on
       if(false == g_Heating->TurnOn())
       {          
		   Logger::Log(F("Unable to turn the heater on"), WAR);          
       }
   }
   else if(JUST_RIGHT == adjustment)
   {       
	   Logger::Log(F("Just Right"), DEB);        
       if(false == DebounceTemperatureReading(JUST_RIGHT))
       {
           goto cleanup;
       }
       //We're in range.  See if we've hit the target temperature and we can stop running alterations
       if(true == g_Heating->IsOn())
       {
           //Heating, see if we're at the target
           if(true == g_InternalFermentorSensor->HaveHitTemperatureTarget(false))
           {
               //We've hit our goal.  See if we can turn it off
               if(false == g_Heating->TurnOff())
               {                   
				   Logger::Log(F("Can't turn the heater off"), WAR);                   
               }
           }
       }
       if(true == g_Cooling->IsOn())
       {
           //Cooling, see if we're at the target
           if(true == g_InternalFermentorSensor->HaveHitTemperatureTarget(true))
           {
               //See if we can turn of the cooling
               if(false == g_Cooling->TurnOff())
               {          
                   //Can't turn the compressor on yet            
				   Logger::Log(F("Can't turn the compressor off"), WAR);
               }
           }
       }
   }
       
cleanup:
   return;
 }


 
  ///<summary>Run through a debounce method to ensure that we're not acting on noise
  ///from the various temperature inputs</summary>
  ///<param name="Temperature">The current temperature band reading</param>
  ///<return>True if we've hit the debounce limit, otherwise false</return>
 bool DebounceTemperatureReading(TempInRange Temperature)
 {
     bool retVal = false;
     if(Temperature != g_LastReading)
     {         
		 Logger::Log(F("Debouncing reset"), DEB);
        
         g_DebounceCounter = 0;       
         g_LastReading = Temperature;
     }
     else 
     {         
         if(g_DebounceCounter + 1 > DEBOUNCE_VALUE)
         {
             
			 Logger::Log(F("Debounce Hit"), DEB);
            
             //We've hit the threshold to break free of our debounce value
             retVal = true;
         }
         else
         {
             //Still working up to hitting the debounce value
             g_DebounceCounter++;			    
			 Logger::PrependLogStatement(DEB);
			 Logger::LogStatement(F("Debounce increasing. Now at "), DEB);
			 Logger::LogStatement(g_DebounceCounter, DEB);
			 Logger::EndLogStatement(DEB);
         }
     }
     return retVal;
 }
 
