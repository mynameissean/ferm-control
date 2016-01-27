// Utility.h

#ifndef _LOGGER_h
#define _LOGGER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include "Definitions.h"

class Logger
{
 private:
	 static ErrorLevels DebugLevel;
 public:
	static void Log(const char*, ErrorLevels);
	static void PrependLogStatement(ErrorLevels);
	static void LogStatement(const char*, ErrorLevels);
	static void LogStatement(byte*, ErrorLevels);
	static void LogStatement(float, ErrorLevels);
	static void EndLogStatement(ErrorLevels);
	static void LogCommunicationStatement(const char*, const char*);
	static void SetLoggingLevel(ErrorLevels LoggingLevel){
		Logger::DebugLevel = LoggingLevel;};
	
};

//extern Utility UTILITY;

#endif

