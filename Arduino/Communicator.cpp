#include "Definitions.h"
#include "Utility.h"
#include "Communicator.h"

/**
 * Create a new communicator socket for receiving and sending commands
 */
Communicator::Communicator()
{
    Serial.setTimeout(DEFAULT_COMM_TIMEOUT);
}

/**
 * Read the serial buffer and see if we have a command from the controller
 * Commands must be terminated with /0 (null terminator)
 * @returns String that was read from the serial buffer, or null if there was an error
 */
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
///<param name="Value">Return value for storing the value of the command (if applicable)</param>
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
#ifdef _DEBUG
         Serial.print("Invalid command: ");
         Serial.println(*Command);
#endif
         goto cleanup;
     }

     //Commands are in the format ACTION_GROUP:VALUE
     //ACTION_GROUP can be any of the following
     //Update temperature target (UTT), followed by a XXX length integer
     //Update temperature band (UTB), followed by a x.xx length float
     //Report current temperature (RCT), followed by a XX length integer indicating the sensor to read
     //Report relay status (RRS), followed by a XX length integer indicating the relay to read
     //Update Relay Status (URS), followed by a XX length integer.  The first   1 indicates off, 2 indicates on
     action = Command->substring(0, Command->indexOf(':'));     

     //Value may not have anything in it.  That's fine for report relay status
      if(true == HasValue(Command))
      {
          value = Command->substring(Command->indexOf(':'), Command->length());
          iValue = value.toInt();
          fValue = value.toFloat();
      }

      if(action.equalsIgnoreCase("UTT"))
      {
          //Update temperature target (UTT), followed by a XXX length integer
          retVal = UTT;
          *IValue = iValue;
      }
      else if(action.equalsIgnoreCase("UTB"))
      {
          //Update temperature band (UTB), followed by a x.xx length float
          retVal = UTB;
          *FValue = fValue;
      }
      else if(action.equalsIgnoreCase("RCT"))
      {
          //Report current temperature (RCT), followed by a XX length integer indicating the sensor to read
          retVal = RCT;
          *IValue = iValue;
      }
      else if(action.equalsIgnoreCase("RRS"))
      {
          //Report relay status (RRS), followed by a XX length integer indicating the relay to read
          retVal = RRS;
          *IValue = iValue;
      }
      else if(action.equalsIgnoreCase("URS"))
      {
          //Update Relay Status (URS), followed by a XX length integer.  The first   1 indicates off, 2 indicates on
          retVal = URS;
          *IValue = iValue;
      }
      else
      {
          //Invalid command
#ifdef _DEBUG
          Serial.print("Unknown command: ");
          Serial.println(*Command);
#endif
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
///and VALUE is up to 4 characters that must parse into either an integer or a float</summary>
///<param name="Command">The command string to validate</param>
///<return> True if valid, false otherwise</return>
bool Communicator::ValidateCommand(String* Command)
{
    bool retVal = false;
    String action;
    String value;
    int iValue = -1;
    float fValue = -1;

    if(NULL == Command || Command->length() < 4 || Command->length() > 8)
    {
        //Invalid
#ifdef _DEBUG
        Serial.print("Invalid string.  Either null or too long: ");
        Serial.println(*Command);
#endif
        goto cleanup;
    }

    //Validate the ACTION_GROUP
    if(-1 == Command->indexOf(':'))
    {
#ifdef _DEBUG
        Serial.print("Invalid string.  No ':' in the command: ");
        Serial.println(*Command);
#endif
        goto cleanup;
    }

    action = Command->substring(0, Command->indexOf(':'));
    if(NULL == action || 3 != action.length())
    {
#ifdef _DEBUG
        Serial.print("Invalid string.  Action group isn't the correct length: ");
        Serial.println(*Command);
#endif
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
    if(NULL == value || 0 == value.length() || value.length() > 4)
    {
#ifdef _DEBUG
        Serial.print("Invalid value.  Length does not match requirements: ");
        Serial.println(value);
#endif
        goto cleanup;
    }

    //Now see if we have either an integer or a float
    iValue = value.toInt();
    fValue = value.toFloat();
    if(0 == iValue && 0.0 == fValue)
    {
#ifdef _DEBUG
        Serial.print("Invalid value: ");
        Serial.println(value);
#endif
        goto cleanup;
    }

    //Success!
    retVal = true;

cleanup:
    return retVal;
}