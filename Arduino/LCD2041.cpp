#include "Definitions.h"
#include "Utility.h"
#include "LCD2041.h"
#include <Wire.h>

#define COMMAND_PREPEND 254


///<summary>Create a new LCD2041 terminal</summary>
///<param name="Address">The physical address of the display terminal</param>
LCD2041::LCD2041(int Address):I2CDisplay(Address)
{
    const int m_MaxNumColumns = 20;
    const int m_MaxNumRows = 2;
    
}

///<summary> Translate the generic command into the specific command code for this
///type of display</summary>
///<param name="Command">The command to send to the display interface</command>
 void LCD2041::SendCommand(I2CDisplay::GenericCommands Command)
{
    switch(Command)
    {
    case(I2CDisplay::BACKLIGHT_ON):
        SendCommand(LCD2041::BACKLIGHT_ON);
        break;
    case(I2CDisplay::BACKLIGHT_OFF):
        SendCommand(LCD2041::BACKLIGHT_OFF);
        break;
    case(I2CDisplay::SET_CURSOR_HOME):
        SendCommand(LCD2041::SET_CURSOR_HOME);
        break;
    case(I2CDisplay::AUTO_SCROLL_ON):
        SendCommand(LCD2041::AUTO_SCROLL_ON);
        break;
    case(I2CDisplay::CLEAR_DISPLAY):
        SendCommand(LCD2041::CLEAR_DISPLAY);
        break;
    }

}

///<summary> Send a command to the LCD2041 display.  All commands are automatically pre-pended
///with a 254 command </summary>
///<param Name="Command">The command to send </param>
void LCD2041::SendCommand(LCD2041::LCD2041Commands Command)
{
    //TODO: 
    //All commands begin with 254
    Wire.write(COMMAND_PREPEND);
	
	//See if this command is going to alter our position
	if(CLEAR_DISPLAY == Command || SET_CURSOR_HOME == Command)
	{
		m_CurrentPosition = 0;		
	}
    Wire.write(Command);
}

///<summary> Send the entire string to be displayed.  It will submit the commands sequentially.
///The string will be drawn on the screen from the current character position. </summary>
///<param Name="DisplayString">The string to display on the LCD screen </param>
///<param Name="Length">How long the string to display is</param>
void LCD2041::SendString(char* DisplayString, int Length)
{    
    for(int i = 0; i < Length; i++)
    {
        int LCD2041DisplayChar = ConvertToLCD2041(DisplayString[i]);
        Wire.write(LCD2041DisplayChar);
		//Change our length position
		m_CurrentPosition++;
		//See if we've wrapped down a line
		while(m_CurrentPosition >= m_MaxNumRows * m_MaxNumColumns)
		{
			//We did, reset it down
			m_CurrentPosition -= m_MaxNumColumns;
		}
    }
}

///<summary>Take in the given character to display and convert it to the
///value that will display on an LCD2041 display</summary>
///<param name="CharacterToDisplay">The character (in ascii) to display on the console.  
///This can only take digits 0-9, :, A-Z, and a-z inclusive.  Other characters will print as a space</param>
///<return>The LCD2041 value for displaying the character, or a generic space in case of an error</return>
int LCD2041::ConvertToLCD2041(char CharacterToDisplay)
{
    int retVal = 0x00;
    //See if this character is a digit
    if(CharacterToDisplay >= '0' && CharacterToDisplay <= '9')
    {
        //Digit.  Subract out the ascii zero to get it down to 0-9 integer values, and then add the 
        //base 0 character to display on screen
        retVal = CharacterToDisplay - '0' + 0x30;
    }
    else if(CharacterToDisplay == ':')
    {
        retVal = 0x3A;
    }
    else if(CharacterToDisplay >= 'A' && CharacterToDisplay <= 'Z')
    {
        retVal = CharacterToDisplay - 'A' + 0x41;
    }
    else if(CharacterToDisplay >= 'a' && CharacterToDisplay <= 'z')
    {
        retVal = CharacterToDisplay - 'a' + 0x61;
    }

    return retVal;
}

///<summary> Reset the position of the cursor to the given Row/Column</summary>
///<param name="Row">The row of the display to advance to, 1 based</param>
///<param name="Column">The column of the display to advance to, 1 based</param>
///<return>True if successful, false if outside of the bounds of the display</param>
bool LCD2041::SetPosition(int Row, int Column)
{
    bool retVal = true;
    if(Row > m_MaxNumRows || Column > m_MaxNumColumns || Row < 0 || Column < 0)
    {
        retVal = false;
        goto cleanup;
    }

    //Valid, advance the cursor
    Wire.write(COMMAND_PREPEND);
    Wire.write(SET_CURSOR_POSITION);
    Wire.write(Column);
    Wire.write(Row);
    
cleanup:
    return retVal;
}

