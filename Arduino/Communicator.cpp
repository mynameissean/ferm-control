#include "Definitions.h"
#include "Utility.h"
#include "Communicator.h"

/**
 * Create a new communicator socket for receiving and sending commands
 */
Communicator::Communicator()
{
    

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