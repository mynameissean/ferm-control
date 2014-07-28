// I2CDisplay.h

#ifndef _I2CDISPLAY_h
#define _I2CDISPLAY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class I2CDisplay
{
protected:
    int m_CurrentPosition;
    static const int m_MaxNumColumns;
    static const int m_MaxNumRows;

private:
    int m_Address;
    bool m_IsValid;

public:
    enum GenericCommands {
        BACKLIGHT_ON = 1,
        BACKLIGHT_OFF,
        SET_CURSOR_HOME,
        AUTO_SCROLL_ON,
        AUTO_SCROLL_OFF,        
        CLEAR_DISPLAY        
    };
    I2CDisplay(int);
    void Disconnect();    
    virtual void SendCommand(GenericCommands) = 0;
    virtual void SendString(char*, int) = 0;
    virtual bool SetPosition(int, int) = 0;
    int GetCurrentCharacterPosition(){return m_CurrentPosition;};
    int GetMaxNumColumns(){return m_MaxNumColumns;}; 
    int GetMaxNumRows(){return m_MaxNumRows;};
    bool IsValid(){return m_IsValid;};
};

#endif