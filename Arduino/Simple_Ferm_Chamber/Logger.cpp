#include "Logger.h"

ErrorLevels Logger::DebugLevel = DEB;


void Logger::PrependLogStatement(ErrorLevels Level)
{
	if(Level <= Logger::DebugLevel)
	{
		switch(Level)
		{
      case COM:
        //Nothing to prepend with
        break;
			case ERR:
				Serial.print("ERR:");
				break;
			case WAR:
				Serial.print("WAR:");
				break;
			case INF:
				Serial.print("INF:");
				break;
			case DEB:
				Serial.print("DEB:");
				break;
			default:
				Serial.print("ERR: Unknown debug level <");
				Serial.print(Level);
				Serial.println(">");
				Serial.print("ERR:");
		}		
	}
}



///<summary>Log out a single line statement.  It does not end the line,
///simply writes out the log to the buffer</summary>
///<param name="Statement">The error message to be logged out</param>
///<param name="Level">What type of message is to be logged</param>
void Logger::LogStatement(byte* Statement, ErrorLevels Level)
{
	if(Level <= Logger::DebugLevel)
	{
		const char* p = reinterpret_cast<const char*>(Statement);
		Serial.print(p);
	}
}

///<summary>Log out a single line statement.  It does not end the line,
///simply writes out the log to the buffer</summary>
///<param name="Statement">The error message to be logged out</param>
///<param name="Level">What type of message is to be logged</param>
void Logger::LogStatement(byte Statement, ErrorLevels Level)
{
	if(Level <= Logger::DebugLevel)
	{
		char p = (char)Statement;
		Serial.print(p);
	}
}

///<summary>Log out a single line statement.  It does not end the line,
///simply writes out the log to the buffer</summary>
///<param name="Statement">The error message to be logged out</param>
///<param name="Level">What type of message is to be logged</param>
void Logger::LogStatement(const __FlashStringHelper* Statement, ErrorLevels Level)
{
	if(Level <= Logger::DebugLevel)
	{		
		Serial.print(Statement);
	}
}

///<summary>Log out a single line statement.  It does not end the line,
///simply writes out the log to the buffer</summary>
///<param name="Statement">The error message to be logged out</param>
///<param name="Level">What type of message is to be logged</param>
void Logger::LogStatement(float Statement, ErrorLevels Level)
{
	if(Level <= Logger::DebugLevel)
	{
		Serial.print(Statement);
	}
}

///<summary>Log out a single line statement.  It does not end the line,
///simply writes out the log to the buffer</summary>
///<param name="Statement">The error message to be logged out</param>
///<param name="Level">What type of message is to be logged</param>
void Logger::LogStatement(int Statement, ErrorLevels Level)
{
	if(Level <= Logger::DebugLevel)
	{
		Serial.print(Statement);
	}
}

///<summary>Log out a single line statement.  It does not end the line,
///simply writes out the log to the buffer</summary>
///<param name="Statement">The error message to be logged out</param>
///<param name="Level">What type of message is to be logged</param>
void Logger::LogStatement(unsigned long Statement, ErrorLevels Level)
{
	if(Level <= Logger::DebugLevel)
	{
		Serial.print(Statement);
	}
}

///<summary>Log out a single line statement.  It does not end the line,
///simply writes out the log to the buffer</summary>
///<param name="Statement">The error message to be logged out</param>
///<param name="Level">What type of message is to be logged</param>
void Logger::LogStatement(const char* Statement, ErrorLevels Level)
{
	if(Level <= Logger::DebugLevel)
	{
		Serial.print(Statement);
	}
}

///<summary> Write out the final character to complete the line of output</summary>
///<param name="Level">What type of statement is being created</param>
void Logger::EndLogStatement(ErrorLevels Level)
{
	if(Level <= Logger::DebugLevel)
	{
		Serial.println();
	}
}

///<summary> Write a complete logging message to the output stream</summary>
///<param name="DebugStatement">What we want to log out</param>
///<param name="Level">What type of message is to be logged</param>
void Logger::Log(const __FlashStringHelper* Statement, ErrorLevels Level)
{
	if(Level <= Logger::DebugLevel)
	{
		PrependLogStatement(Level);			
		LogStatement(Statement, Level);
		EndLogStatement(Level);
	}

}

///<summary> Write a complete logging message to the output stream</summary>
///<param name="DebugStatement">What we want to log out</param>
///<param name="Level">What type of message is to be logged</param>
void Logger::Log(const char* Statement, ErrorLevels Level)
{
	if(Level <= Logger::DebugLevel)
	{
		PrependLogStatement(Level);			
		LogStatement(Statement, Level);
		EndLogStatement(Level);
	}

}

///<summary>Write a communication statement up to the control program.  It will
///be written as Command:Value</summary>
///<param name="Command">The command structure to send</param>
///<param name="Value">The value of the command order</param>
void Logger::LogCommunicationStatement(const char* Command, const char* Value)
{
	if(COM <= Logger::DebugLevel)
	{
		PrependLogStatement(COM);
		LogStatement(Command, COM);
		LogStatement(":", COM);
		LogStatement(Value, COM);
		EndLogStatement(COM);
	}
}
