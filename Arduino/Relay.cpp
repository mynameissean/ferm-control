///<summary>This class describes a relay on the system. It can have either
///power going through it or not.</summary>

#include "Relay.h"
#include "Definitions.h"
#include "Utility.h"
#define _DEBUG 5
///<summary>Create a relay that is able to activate the given pin output</summary>
///<param name="Pin">The wire on the arduino board to activate</param>
///<param name="DIsplayPin">The wire on the arudiono board to activate for displaying the relay is active</param>
Relay::Relay(int Pin, int DisplayPin)
{
    m_TriggerPin = Pin;
    m_DisplayPin = DisplayPin;
    m_MinRunTime = INVALID_TIME;
    m_CompressorDelay = INVALID_TIME;
    m_OffTimeStart = INVALID_TIME;
    m_OnTimeStart = INVALID_TIME;

    //Make sure the relay's off
    TurnOff();

}

///<summary>Create a relay that is able to activate the given pin output
///This relay will activate for a given minimum amount of time, and will not
///reactivate after being turned off for a given amount of time.  This is used
///to prevent excessive compressor cycling and inhibit damage to the system.</summary>
///<param name="Pin">The wire on the arduino board to activate</param>
///<param name="DIsplayPin">The wire on the arudiono board to activate for displaying the relay is active</param>
///<param name="MinRunTime">The minimum amount of time to run the relay, measured in ms</param>
///<param name="CompressorDelay">How long to wait before activating the relay after turning it off, measured in ms</param>
Relay::Relay(int TriggerPin, int DisplayPin, unsigned long MinRunTime, unsigned long CompressorDelay)
{
    m_TriggerPin = TriggerPin;
    m_DisplayPin = DisplayPin;
    m_MinRunTime = MinRunTime;
    m_CompressorDelay = CompressorDelay;
    m_OffTimeStart = INVALID_TIME;
    m_OnTimeStart = INVALID_TIME;
    

    //Make sure the relay's off
    TurnOff();
}

///<summary>Turn on the relay if it's possible to.</summary>
///<return>True if successful, false otherwise. Currently cannot fail.</return>
bool Relay::TurnOn()
{
#ifdef _DEBUG
    Serial.print("Turning on relay");
    Serial.println(m_TriggerPin);
#endif
    digitalWrite(m_TriggerPin, HIGH);
    if(INVALID_PIN != m_DisplayPin)
    {
        digitalWrite(m_DisplayPin, HIGH);
    }
    m_IsOn = true;

    //Set the time we turned on
    m_OnTimeStart = millis();
    return true;
}

///<summary>Turn off the relay if it's possible to.</summary>
///<return>True if successful, false otherwise.  Currently cannot fail.</return>
bool Relay::TurnOff()
{
#ifdef _DEBUG
    Serial.print("Turning off relay");
    Serial.println(m_TriggerPin);
#endif
    digitalWrite(m_TriggerPin, LOW);
    //See if we have a display pin to work with
    if(INVALID_PIN != m_DisplayPin)
    {
        digitalWrite(m_DisplayPin, LOW);
    }
    m_IsOn = false;

    //Set the time we turned off
    m_OffTimeStart = millis();
    return false;
}



///<summary>Determine if it's possible to turn on the relay.</summary>
///<return>True if the relay can be turned on, false otherwise</return>
bool Relay::CanTurnOn()
{
    bool retVal = false;
    unsigned long difference = 0;    
    //See if we're already on
    if(true == m_IsOn)
    {
        //Already on.  We can't get any more on
        goto cleanup;
    }

    //See if we have a minimum wait time
    if(INVALID_TIME == m_CompressorDelay)
    {
        //We don't need to wait to turn this on
        retVal = true;
        goto cleanup;
    }

    //Get the current time difference
    difference = Utility::TimeDifference(m_OffTimeStart);

    //See if we've exceeded the necessary wait limit
    if(difference > m_CompressorDelay)
    {
        //We've waited long enough, turn on the relay
        retVal = true;
        goto cleanup;
    }

cleanup:
    return retVal;
}


///<summary>Determine if it's possible to turn off the relay.</summary>
///<return>True if the relay can be turned off, false otherwise</return>
bool Relay::CanTurnOff()
{
    bool retVal = false;
    unsigned long difference = 0;

    if(false == m_IsOn)
    {
        //Already off.  We can't get any more off
        goto cleanup;
    }

    //See if we have a minimum wait time
    if(INVALID_TIME == m_MinRunTime)
    {
        //We don't need to wait to turn this on
        retVal = true;
        goto cleanup;
    }

    //Get the current time difference
    difference = Utility::TimeDifference(m_OnTimeStart);

    //See if we've exceeded the necessary wait limit
    if(difference > m_MinRunTime)
    {
        //We've waited long enough, turn on the relay
        retVal = true;
        goto cleanup;
    }

cleanup:
    return retVal;
}

