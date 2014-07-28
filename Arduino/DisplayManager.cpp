#include "Definitions.h"
#include "Utility.h"
#include "DisplayManager.h"
#include "LCD2041.h"

///<summary>Create a new manager for keeping track of how and what to send to
///our output terminal</summary>
///<param name="Address">The phsyical address for the display</param>
///<param name="Type">What type of display we have on our system</param>
DisplayManager::DisplayManager(int Address, DisplayTypes Type)
{
	if(Type == LCD2041)
	{
		m_Display = new LCD2041::LCD2041(Address);
	}
}

///<summary> Given the array of TemperatureSensors, print out their friendly name
///and their current temperature</summary>
///<param name="Sensors">Collection of temperature sensors that are to be displayed</summary>
///<param name="Length">How many temperature sensors we want to display</param>
void DisplayManager::ShowTemperature(TemperatureSensor** Sensors, int Length)
{
    char* display = NULL;

    //Clear the display
    m_Display->SendCommand(I2CDisplay::CLEAR_DISPLAY);
    
    if(Length > m_Display->GetMaxNumRows())
    {
        //Can't display these sensors.  Just do the first ones
        #ifdef _DEBUG
        Serial.println("More than the maximum number of sensors asked to display.");
        #endif
        Length = m_Display->GetMaxNumRows();
    }

	for(int i = 0; i < Length; i++)
    {
        //Prepare us to print to the next line
        m_Display->SetPosition(0, i + 1);

        //Count how many characters we're trying to print
        //Take the name, plus 1 for a colon, and 5 for the temperature (Primary:198.6)
        int count = Sensors[i]->GetFriendlyNameLength() + 6;
        if(count > m_Display->GetMaxNumColumns())
        {
            #ifdef _DEBUG
            Serial.println("Maximum line length exceeded.  Trimming name");
            #endif

            Sensors[i]->TrimNameToLength(m_Display->GetMaxNumColumns() - 5);
            count = m_Display->GetMaxNumColumns();
        }

        display = new char[count];
        sprintf(display,"%s:4.1f", Sensors[i]->GetFriendlyName(), Sensors[i]->GetTemperature());       
        
        //Now that we have the line we want, display it
        m_Display->SendString(display, count - 1);        
    }
}

///<summary>Display a string to the display</summary>
///<param name="StringToDisplay>The string we want displayed on our terminal</param>
///<param name="StrLength">How long the string we want to display is</param>
///<param name="Clear">True means to clear the display first.  False means leave it as is</param>
///<param name="SetToBeginningOfLine">Move the cursor to the beginning of the line it is currently on.  This is
///destructive and will overwrite what is currently on the characters</param>
void DisplayManager::ShowString(char* StringToDisplay, int StrLength, bool Clear, bool SetToBeginningOfLine)
{

		if(true == Clear)
		{
			//Clear the display
            m_Display->SendCommand(I2CDisplay::CLEAR_DISPLAY);
		}
		
		//See if we need to set to the beginning of the line
		/*if(true == SetToBeginningOfLine)
		{
			if(m_Display->GetCurrentCharacterPosition() % GetMaxLineLength != 0)
			{
				//We're not at the beginning of a line
			}
		}
		
		//Show the string.  We'll need to split it up across multiple lines if requested
		int lineCount = 0;
		int position = 0;
		while(position < StrLength)
		{
			for(int i = 0; i < lineCount && postion < StrLength; i++)
			{
				//Get the character we're going to display
			}
		}*/
		
}