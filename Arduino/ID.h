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

public:
    ID(char*, int, int);
    char* GetName(){return m_FriendlyName;};
    int GetNameLength(){return m_FriendlyNameLength;};
    int GetIndex(){return m_Index;};
};

#endif