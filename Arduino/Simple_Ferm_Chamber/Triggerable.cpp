#include "Definitions.h"
#include "Triggerable.h"

///<summary>Create a new triggerable event</summary>
///<param name="InTriggerPin">The pin to trigger depending on the command</param>
Triggerable::Triggerable(int InTriggerPin)
{
    m_TriggerPin = InTriggerPin;
    m_DisplayPin = INVALID_PIN;
}

///<summary>Create a new triggerable event with a display option</summary>
///<param name="InTriggerPin">The pin to trigger depending on the command</param>
///<param name="InDisplayPin">The pin to trigger to display</param>
Triggerable::Triggerable(int InTriggerPin, int InDisplayPin)
{
    m_TriggerPin = InTriggerPin;
    m_DisplayPin = InDisplayPin;
}


///<summary>Turn the pin high or low depending on the input parameter</summary>
///<param name="Setting">The value to set the trigger pin and display pin(if applicable
/// to</param>
void Triggerable::Trigger(int Setting)
{
    #ifdef _DEBUG
    Serial.print("Turning off triggerable");
    Serial.println(m_TriggerPin);
#endif
    digitalWrite(m_TriggerPin, Setting);
    //See if we have a display pin to work with
    if(INVALID_PIN != m_DisplayPin)
    {
        digitalWrite(m_DisplayPin, Setting);
    }
}

///<summary> Trigger the various pins high </summary>
void Triggerable::TriggerHigh()
{
    Trigger(HIGH);
}

///<summary> Trigger the various pins low </summary>
void Triggerable::TriggerLow()
{
    Trigger(LOW);
}