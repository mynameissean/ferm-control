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
   bool ValidateCommand(String*);
   bool HasValue(String*);
public:
    Communicator();
    String* Read();
    OperatingCommand ParseCommand(String*, int*, float*);
};
#endif