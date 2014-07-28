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
#define _DEBUG 5
int TempReadPin = 3;
int CoolingRelayPin = 4;
int CoolingDisplayPin = 5;
int HeatingDisplayPin = 6;
int HeatingRelayPin = 7;
int StatusLED = 13;
#define DEBOUNCE_VALUE 5 //TODO: make 5
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
float g_PrimaryTargetTemperature = 70;
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
  GatherTemperatureData();

  //Step 3: Compare them to our values and adjust
  AdjustPrimaryTemperature();

  //Step 4: See if we need to store our current temperature data
  //SaveTemperatureData();
  
  //Signal that we're done with this cycle
  Utility::Cycle(StatusLED, 1000, 1000);
  delay(CYCLE_TIME);
 }

 /**
  * Open up communication with our controller and see if there are any new commands to receive
  */
 void ReceiveOperatingInstructions()
 {
     String* command = m_Communicator->Read();
     if(NULL == command || 0 == command->length())
     {
         //Nothing to parse
         goto cleanup;
     }

     //We have a command
     ParseCommand(command);

cleanup:
     return;
 }

 /** 
  * Parse out the command and perform the requested actions
  * @param Command The command to act on from the controller
  */
 void ParseCommand(String* Command)
 {
     String value;

     //Commands are in the format ACTION_GROUP:VALUE
     //ACTION_GROUP can be any of the following
     //Update temperature target (UTT), followed by a XXX length integer
     //Update temperature band (UTB), followed by a x.xx length float
     //Report current temperature (RCT), followed by a XX length integer indicating the sensor to read
     //Report relay status (RRS)
     String action = Command->substring(0, Command->indexOf(':'));
     if(0 == action.length())
     {
         //Invalid command
         goto cleanup;
     }

      value = Command->substring(Command->indexOf(':'));
     //Value may not have anything in it.  That's fine for report relay status

     

cleanup:
     return;

 }

 /**
  * Gather the temperature data from the thermistors in the system
  */
 void GatherTemperatureData()
 {
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
   return;
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
       //See if we can turn off the heating if it's already on
       if(true == g_Heating->IsOn())
        {
           if(false == g_Heating->CanTurnOff())
           {
               //Unable to turn the heater off while it's already running
               #ifdef _DEBUG 
                    Serial.println("Can't turn the heater off");
                #endif
               goto cleanup;
           }
           else {
               //Turn the heating off
               #ifdef _DEBUG 
                    Serial.println("Heating");
                #endif
               g_Heating->TurnOff();
           }
       }
       //We need to see if we can activate the cooling
       if(false == g_Cooling->IsOn())
       {
           if(false == g_Cooling->CanTurnOn())
           {
               //Can't turn the compressor on yet
               #ifdef _DEBUG 
                    Serial.println("Can't turn the compressor on");
                #endif
                Utility::Cycle(g_Cooling->GetDisplayPin(), 250, 250);
               goto cleanup;
           }
           else {
               //Turn on the cooling
               #ifdef _DEBUG 
                   Serial.println("Cooling");
               #endif
               g_Cooling->TurnOn();
           }
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
        if(true == g_Cooling->IsOn())
        {
           if(false == g_Cooling->CanTurnOff())
           {
               //Can't turn it off yet
               goto cleanup;
           }
           else
           {
               #ifdef _DEBUG 
                  Serial.println("Cooling");
               #endif
               g_Cooling->TurnOff();
           }
        }
       //We need to see if we can activate the heating
        if(false == g_Heating->IsOn())
        {
           if(false == g_Heating->CanTurnOn())
           {
               //Can't turn the heater on yet
               Utility::Cycle(g_Heating->GetDisplayPin(), 250, 250);
               goto cleanup;
           }
           else {
               //Turn on the heating
               #ifdef _DEBUG 
                  Serial.println("Heating");
               #endif
               g_Heating->TurnOn();
           }       
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
               if(true == g_Heating->CanTurnOff())
               {
                   #ifdef _DEBUG 
                      Serial.println("Heating");
                   #endif
                   g_Heating->TurnOff();
               }
           }
       }
       if(true == g_Cooling->IsOn())
       {
           //Cooling, see if we're at the target
           if(true == g_PrimarySensor->HaveHitTemperatureTarget(true))
           {
               //We've hit our goal.  See if we can turn it off
               if(true == g_Cooling->CanTurnOff())
               {
                   #ifdef _DEBUG 
                      Serial.println("Cooling");
                   #endif
                   g_Cooling->TurnOff();
               }
           }
       }
   }
       
cleanup:
   return;
 }


 /**
  * Run through a debounce method to ensure that we're not acting on noise
  * from the various temperature inputs
  */
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
 
