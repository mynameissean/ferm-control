// Definitions.h

#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#define INVALID_DATA 1000
#define INVALID_TIME 0
#define INVALID_PIN -1
#define UNSIGNED_LONG_MAX_VALUE 4294967295 //(2^32-1)
#define MAX_COMMAND_LENGTH 30
#define DEFAULT_COMM_TIMEOUT 1000

enum TempInRange {TOO_HOT, TOO_COLD, JUST_RIGHT, NO_READING };
enum OperatingCommand {UTT, UTB, RCT, RSI, RRS, RRI, URS, HBS, INVALID };
//#define _DEBUG 1

//extern Utility UTILITY;

#endif

