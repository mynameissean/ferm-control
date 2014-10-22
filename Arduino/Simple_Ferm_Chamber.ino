#include <Wire.h>
#include "Relay.h"
#include "Utility.h"
#include "TemperatureSensor.h"
#include "Definitions.h"
#include <OneWire.h>
#include "DisplayManager.h"
#include "Communicator.h"

/*
 * This is a stripped down version of the fermentation controller
 * project.  It doesn't allow for command and control via
 * external software program.  It only sends output out to the 
 * wire for monitoring purposes.
 */
#define _DEBUG 0
int TempReadPin = 3;
int CoolingRelayPin = 4;
int CoolingDisplayPin = 5;
int HeatingDisplayPin = 6;
int HeatingRelayPin = 7;
int StatusLED = 13;
#define DEBOUNCE_VALUE 5
#define CYCLE_TIME 5000


//Global objects
OneWire g_TempSensors(TempReadPin);
byte g_PrimarySensorAddress[8] = {40,126,182,231,3,0,0,109};
byte g_ambientExternalSensorAddress[8] = {40,118,221,231,3,0,0,173};
byte g_ambientInternalSensorAddress[8] = {40,126,182,231,3,0,0,109};
TemperatureSensor* g_PrimarySensor;
TemperatureSensor* g_AmbientExternalSensor;
TemperatureSensor* g_AmbientInternalSensor;
Relay* g_Cooling;
Relay* g_Heating;
TempInRange g_LastReading = JUST_RIGHT;
int g_DebounceCounter = 0;
DisplayManager* m_LCDDisplay = NULL;
Communicator* m_Communicator = NULL;

//Setup values
float g_PrimaryTemperatureBand = 1.3;
float g_PrimaryTargetTemperature = 67;
unsigned long g_CompressorRunTime = 30000; //30 Seconds
unsigned long g_CompressorOffTime = 240000; //4 Minutes
unsigned long g_HeatingOffTime = 300000;    //5 Minutes

void setup(){
   Serial.begin(9600);
   pinMode(StatusLED, OUTPUT); 
   pinMode(CoolingDisplayPin, OUTPUT);
   pinMode(HeatingDisplayPin, OUTPUT);
   pinMode(CoolingRelayPin, OUTPUT);
   pinMode(HeatingRelayPin, OUTPUT);

   //Setup our temperature sensors
   g_PrimarySensor = new TemperatureSensor(g_PrimarySensorAddress, g_PrimaryTemperatureBand, g_PrimaryTargetTemperature, "Primary", strlen("Primary"));
   g_AmbientExternalSensor = new TemperatureSensor(g_ambientExternalSensorAddress, g_PrimaryTemperatureBand, g_PrimaryTargetTemperature, "Secondary", strlen("Secondary"));
   g_AmbientInternalSensor = new TemperatureSensor(g_ambientInternalSensorAddress, g_PrimaryTemperatureBand, g_PrimaryTargetTemperature, "Ambient", strlen("Ambient"));

   //Setup our relays
   g_Cooling = new Relay(CoolingRelayPin, CoolingDisplayPin, g_CompressorRunTime, g_CompressorOffTime); //4 minutes
   g_Heating = new Relay(HeatingRelayPin, HeatingDisplayPin, 0, g_HeatingOffTime);

   //Setup our display if we have one
   //Wire.begin();
   //m_LCDDisplay = new DisplayManager(0x2E, DisplayManager::LCD2041);

   //Setup our controller (if applicable)
   m_Communicator = new Communicator();

   //Signal that we're powered on and ready
   Utility::Cycle(g_Cooling->GetDisplayPin(), 1000, 1000);
   Utility::Cycle(g_Heating->GetDisplayPin(), 1000, 1000);
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
          Utility::Cycle(g_Heating->GetDisplayPin(), 1000, 1000);
          Utility::Cycle(g_Cooling->GetDisplayPin(), 1000, 1000);
      }
      goto cleanup;
  }

  //Step 3: Compare them to our values and adjust
  AdjustPrimaryTemperature();

  //Step 4: See if we need to store our current temperature data
  //SaveTemperatureData();
  
cleanup:
  //Signal that we're done with this cycle
  Utility::Cycle(StatusLED, 1000, 1000);
  delay(CYCLE_TIME);
 }

 /**
  * Open up communication with our controller and see if there are any new commands to receive
  */
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
   if(false == g_PrimarySensor->DoesSensorExist(g_TempSensors))
   {
     Serial.println("Unable to find sensor.");
     goto cleanup;
   }
   //Found the sensor we wanted, get the temperature data   
   
   if(INVALID_DATA == g_PrimarySensor->RetrieveTemperatureFromSensor(g_TempSensors))
   {  
     Serial.println("Unable to get temperature data for primary sensor.");
     goto cleanup;
   }

   //Got the one we need.  The rest are secondary
   retVal = true;
   if(true == g_AmbientExternalSensor->DoesSensorExist(g_TempSensors))
   {
     //Get the temperature
     g_AmbientExternalSensor->RetrieveTemperatureFromSensor(g_TempSensors);
   }
   
   if(true == g_AmbientInternalSensor->DoesSensorExist(g_TempSensors))
   {
     //Get the temperature
     g_AmbientInternalSensor->RetrieveTemperatureFromSensor(g_TempSensors);
   }
     
#ifdef _DEBUG
   //Print out the temperature
   Serial.print("Primary Temperature: ");
   Serial.println(g_PrimarySensor->GetTemperature());
   Serial.print("Ambient Extneral Temperature: ");
   Serial.println(g_AmbientExternalSensor->GetTemperature());
   Serial.print("Ambient Internal Temperature: ");
   Serial.println(g_AmbientInternalSensor->GetTemperature());
#endif
   
cleanup:
   return retVal;
 }

 
 /**
  * Determine if any of our temperature sensors is outside of the necessary ranges.
  */
 void AdjustPrimaryTemperature()
 {
   //We care most about the primary temperature.  This is the temperature we want to control
   TempInRange adjustment = g_PrimarySensor->ShouldBeginTemperatureAdjustment();
   
   if(TOO_HOT == adjustment)
   {
#ifdef _DEBUG 
       Serial.println("Too hot");
#endif
       if(false == DebounceTemperatureReading(TOO_HOT))
       {
           goto cleanup;
       }
       //Turn the heater off if it's on
       if(false == g_Heating->TurnOff())
       {
          #ifdef _DEBUG
           Serial.println("Unable to turn the heater off");
          #endif
       }
       //We need to see if we can activate the cooling
       if(false == g_Cooling->TurnOn())
       {          
               //Can't turn the compressor on yet
#ifdef _DEBUG 
        Serial.println("Can't turn the compressor on");
#endif
        }
   }
   else if(TOO_COLD == adjustment)
   {
       #ifdef _DEBUG 
        Serial.println("Too cold");
        #endif
        if(false == DebounceTemperatureReading(TOO_COLD))
        {
            goto cleanup;
        }
       //See if we can turn of the cooling
        if(false == g_Cooling->TurnOff())
       {          
               //Can't turn the compressor on yet
#ifdef _DEBUG 
        Serial.println("Can't turn the compressor off");
#endif
        }
       //Turn the heater off if it's on
       if(false == g_Heating->TurnOn())
       {
          #ifdef _DEBUG
           Serial.println("Unable to turn the heater on");
          #endif
       }
   }
   else if(JUST_RIGHT == adjustment)
   {
       #ifdef _DEBUG 
            Serial.println("Just Right");
        #endif
       if(false == DebounceTemperatureReading(JUST_RIGHT))
       {
           goto cleanup;
       }
       //We're in range.  See if we've hit the target temperature and we can stop running alterations
       if(true == g_Heating->IsOn())
       {
           //Heating, see if we're at the target
           if(true == g_PrimarySensor->HaveHitTemperatureTarget(false))
           {
               //We've hit our goal.  See if we can turn it off
               if(false == g_Heating->TurnOff())
               {
                   #ifdef _DEBUG 
                      Serial.println("Can't turn the heater off");
                   #endif                   
               }
           }
       }
       if(true == g_Cooling->IsOn())
       {
           //Cooling, see if we're at the target
           if(true == g_PrimarySensor->HaveHitTemperatureTarget(true))
           {
               //See if we can turn of the cooling
               if(false == g_Cooling->TurnOff())
               {          
                       //Can't turn the compressor on yet
            #ifdef _DEBUG 
                    Serial.println("Can't turn the compressor off");
            #endif
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
         #ifdef _DEBUG 
            Serial.println("Debouncing reset");
        #endif
         g_DebounceCounter = 0;       
         g_LastReading = Temperature;
     }
     else 
     {         
         if(g_DebounceCounter + 1 > DEBOUNCE_VALUE)
         {
             #ifdef _DEBUG 
                Serial.println("Debounce Hit");
            #endif
             //We've hit the threshold to break free of our debounce value
             retVal = true;
         }
         else
         {
             //Still working up to hitting the debounce value
             g_DebounceCounter++;

             #ifdef _DEBUG 
                Serial.print("Debounce Increasing.  Now at: ");
                Serial.println(g_DebounceCounter);
            #endif
             
         }
     }
     return retVal;
 }
 
