#include "Definitions.h"
#include "Utility.h"
#include "Communicator.h"

/**
 * Create a new communicator socket for receiving and sending commands
 */
Communicator::Communicator()
{
    m_CharBuffer = new char[MAX_COMMAND_LENGTH];

}

char* Communicator::Read()
{


    if(Serial.available() > 0)
    {
        //Let's see if we have a full string now
        String command = Serial.readStringUntil('/0');
        
    }
}