#ifndef _LCD2041_h
#define _LCD2041_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "I2CDisplay.h"

class LCD2041 : public I2CDisplay
{    	
private:
    enum LCD2041Commands {
        BACKLIGHT_ON = 66,
        BACKLIGHT_OFF = 70,
        SET_CURSOR_POSITION = 71,
        SET_CURSOR_HOME = 72,
        AUTO_SCROLL_ON = 81,
        //AUTO_SCROLL_OFF = 82,        
        CLEAR_DISPLAY = 88
    };
    void SendCommand(LCD2041Commands);
    int ConvertToLCD2041(char);

public:
    LCD2041(int);
    void SendCommand(I2CDisplay::GenericCommands);    
    void SendString(char*, int);
	bool SetPosition(int, int);


 

};

#endif