#include <Windows.h>
#include "Debug.h"

char* ToString(int value)
{
	static char tempString[16];
	sprintf_s<sizeof(tempString)>(tempString, "%i", value);

	return tempString;
}

void Debug::Log(std::string message, DebugLevel level, int lineNumber, const char* filename)
{
	std::string debugMessage = "";

	if (filename != nullptr)
	{
		debugMessage = '[';
		debugMessage += filename;
		debugMessage += "] ";
 	}
	if (lineNumber > 0)
	{
		debugMessage += '('; 
		debugMessage += lineNumber;
		debugMessage += ") ";
	}
	switch (level)
	{
	case DebugLevel::Log:
		debugMessage += "Log: ";
		break;
	case DebugLevel::Warning:
		debugMessage += "Warning: "; 
		break;
	case DebugLevel::Error:
	case DebugLevel::Fatal:
		debugMessage += "Error: ";
		break;
	default:
		break;
	}

	debugMessage += message + "\n";

	OutputDebugStringA(debugMessage.c_str());
	//printf(debugMessage.c_str());
}