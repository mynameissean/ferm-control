#ifndef _ID_h
#define _ID_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class ID
{    	
private:    
    char* m_FriendlyName;
    int m_FriendlyNameLength;
    int m_Index;
    int m_EEPROMAddress;

public:
    ID(char*, int, int);
    ID(char*, int, int, int);
    char* GetName(){return m_FriendlyName;};
    int GetNameLength(){return m_FriendlyNameLength;};
    int GetIndex(){return m_Index;};
    int GetEEPROMAddress(){return m_EEPROMAddress;};
};

#endif