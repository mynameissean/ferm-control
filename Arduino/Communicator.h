#ifndef _COMMUNICATOR_H
#define _COMMUNICATOR_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class Communicator
{
private:   
   void ClearReceiveBuffer();

public:
    Communicator();
    String* Read();
};
#endif