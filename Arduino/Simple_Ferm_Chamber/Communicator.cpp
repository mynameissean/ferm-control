#include "Definitions.h"
#include "Utility.h"
#include "Communicator.h"
#include "Logger.h"

///<summary>Create a new communicator socket for receiving and sending commands</summary>
Communicator::Communicator()
{
    Serial.setTimeout(DEFAULT_COMM_TIMEOUT);
}

///<summary>Read the serial buffer and see if we have a command from the controller
///Commands must be terminated with /0 (null terminator</summary>
///<return>returns String that was read from the serial buffer, or null if there was an error</return>
String* Communicator::Read()
{
    String* retVal = NULL;

    if(Serial.available() > 0)
    {
        //Let's see if we have a full string now
        *retVal = Serial.readStringUntil('/0');
        if(0 == retVal->length())
        {
            //Don't have the full command yet
            retVal = NULL;
            goto cleanup;
        }        
    }

cleanup:
    return retVal;
}

///<summary>Parse the command from the communication string</summary>
///<param name="IValue">Return value for storing the intger value of the command (if applicable)</param>
///<param name="FValue">Return value for storing the float value of the command (if applicable)</param>
///<return>Operating command that was parsed out</return>
OperatingCommand Communicator::ParseCommand(String* Command, int* IValue, float* FValue)
{
     OperatingCommand retVal = INVALID;
     String action;
     String value;
     int iValue;
     float fValue;
     if(false == ValidateCommand(Command))
     {
		//Write out a warning to our output stream
		Logger::PrependLogStatement(WAR);
		Logger::LogStatement(F("Invalid Command <"), WAR);
		Logger::LogStatement(Command->c_str(), WAR);	
		Logger::LogStatement(F(">"), WAR);
		Logger::EndLogStatement(WAR);
        goto cleanup;
     }

     //Commands are in the format ACTION_GROUP:VALUE
	 //For certain ACTION_GROUP commands, the value is not needed, but the : is still required
     //ACTION_GROUP can be any of the following
     //Update temperature target (UTT), followed by a XXX.X length float
     //Update temperature band (UTB), followed by a xx.xx length float
     //Report sensor index (RSI), followed by a XX length integer indicating the sensor to read
     //Report current temperature (RCT), followed by a XX length integer indicating the sensor to read
     //Report relay index (RRI), followed by a XX length integer indicating the sensor to read
     //Report relay status (RRS), followed by a XX length integer indicating the relay to read
     //Update Relay Status (URS), followed by a XX length integer.  The first   1 indicates off, 2 indicates on
	 //List Temp Sensors (LTS). This will cause the arduino to list out the sensors that are attached
	 //Heart Beat Service (HBS).  Will force the controller to respond back that it is still active
     action = Command->substring(0, Command->indexOf(':'));     

     //Value may not have anything in it.  That's fine for report temp sensors
      if(true == HasValue(Command))
      {
          value = Command->substring(Command->indexOf(':'), Command->length());
          iValue = value.toInt();

		  char buffer[10];
		  value.toCharArray(buffer, 10);
          fValue = atof(buffer);
      }

      if(action.equalsIgnoreCase(F("UTT")))
      {
          //Update temperature target (UTT), followed by a XXX length integer
          retVal = UTT;
          *FValue = fValue;
      }
      else if(action.equalsIgnoreCase(F("UTB")))
      {
          //Update temperature band (UTB), followed by a x.xx length float
          retVal = UTB;
          *FValue = fValue;
      }
      else if(action.equalsIgnoreCase(F("RCT")))
      {
          //Report current temperature (RCT), followed by a XX length integer indicating the sensor to read
          retVal = RCT;
          *IValue = iValue;
      }
      else if(action.equalsIgnoreCase(F("RRS")))
      {
          //Report relay status (RRS), followed by a XX length integer indicating the relay to read
          retVal = RRS;
          *IValue = iValue;
      }
      else if(action.equalsIgnoreCase(F("URS")))
      {
          //Update Relay Status (URS), followed by a XX length integer.  The first   1 indicates off, 2 indicates on
          retVal = URS;
          *IValue = iValue;
      }
      else if(action.equalsIgnoreCase(F("RSI")))
      {
          //Report sensor index (RSI), followed by a XX length integer indicating the sensor to read
          retVal = RSI;
          *IValue = iValue;
      }
      else if(action.equalsIgnoreCase(F("RRI")))
      {
          //Report relay index (RRI), followed by a XX length integer indicating the sensor to read
          retVal = RRI;
          *IValue = iValue;
      }
      else if(action.equalsIgnoreCase(F("HBS")))
      {
          //Heart beat service (HBS)
          retVal = HBS;
      }	  
	  else if (action.equalsIgnoreCase(F("LTS")))
	  {
		  //List temperature sensors (LTS).  Just return out the sensors that we have attached
		  retVal = LTS;
	  }
      else
      {
          //Invalid command
		  Logger::PrependLogStatement(WAR);
		  Logger::LogStatement(F("Unknown Command <"), WAR);
		  Logger::LogStatement(Command->c_str(), WAR);	
		  Logger::LogStatement(F(">"), WAR);
		  Logger::EndLogStatement(WAR);
          retVal = INVALID;
      }

     

cleanup:
      return retVal;
}

///<summary>See if this command has a value string along with it</summary>
///<param name="Command">The command to parse</param>
///<return>True if there's a possible value, false otherwise</return>
bool Communicator::HasValue(String* Command)
{
    bool retVal = false;

    String value = Command->substring(Command->indexOf(':'), Command->length());
    if(NULL != value && value.length() > 1)
    {
        retVal = true;
    }

    return retVal;
}

///<summary>Validate that the given command is the proper format and structure.
///Commands are of the format ACTION_GROUP:VALUE, where ACTION_GROUP is 3 characters 
///and VALUE is up to 6 characters that must parse into either an integer or a float</summary>
///<param name="Command">The command string to validate</param>
///<return> True if valid, false otherwise</return>
bool Communicator::ValidateCommand(String* Command)
{
    bool retVal = false;
    String action;
    String value;
    int iValue = -1;
    float fValue = -1;

    if(NULL == Command || Command->length() < 4 || Command->length() > 11)
    {
        //Invalid
		Logger::PrependLogStatement(WAR);
		Logger::LogStatement(F("Invalid string.  Either null or too long <"), WAR);
		Logger::LogStatement(Command->c_str(), WAR);	
		Logger::LogStatement(F(">"), WAR);
		Logger::EndLogStatement(WAR);
        goto cleanup;
    }

    //Validate the ACTION_GROUP
    if(-1 == Command->indexOf(':'))
    {        
		Logger::PrependLogStatement(WAR);
		Logger::LogStatement(F("Invalid string.  No 'colon' in the command <"), WAR);
		Logger::LogStatement(Command->c_str(), WAR);	
		Logger::LogStatement(F(">"), WAR);
		Logger::EndLogStatement(WAR);
        goto cleanup;
    }

    action = Command->substring(0, Command->indexOf(':'));
    if(NULL == action || 3 != action.length())
    {
		Logger::PrependLogStatement(WAR);
		Logger::LogStatement(F("Invalid string.  Action group isn't the correct length <"), WAR);
		Logger::LogStatement(Command->c_str(), WAR);	
		Logger::LogStatement(F(">"), WAR);
		Logger::EndLogStatement(WAR);
        goto cleanup;
    }

    //See if we have a value to work with too
    if(false == HasValue(Command))
    {
        //No value, we're a good format.  
        retVal = true;
        goto cleanup;
    }

    //We have a value to parse.  Let's see if it's a valid int or float
    value = Command->substring(Command->indexOf(':'), Command->length());
    if(NULL == value || 0 == value.length() || value.length() > 6)
    {
		Logger::PrependLogStatement(WAR);
		Logger::LogStatement(F("Invalid value.  Length does not match requirements <"), WAR);
		Logger::LogStatement(value.c_str(), WAR);	
		Logger::LogStatement(F(">"), WAR);
		Logger::EndLogStatement(WAR);
        goto cleanup;
    }

    //Now see if we have either an integer or a float
    iValue = value.toInt();

	char buffer[10];
	value.toCharArray(buffer, 10);

    fValue = atof(buffer);
    if(0 == iValue && 0.0 == fValue)
    {
		Logger::PrependLogStatement(WAR);
		Logger::LogStatement(F("Invalid value. <"), WAR);
		Logger::LogStatement(value.c_str(), WAR);	
		Logger::LogStatement(F(">"), WAR);
		Logger::EndLogStatement(WAR);
        goto cleanup;
    }

    //Success!
    retVal = true;

cleanup:
    return retVal;
}
