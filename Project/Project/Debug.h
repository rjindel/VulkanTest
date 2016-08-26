#pragma once
#include <string>

enum class DebugLevel
{
	Verbose,
	Informational,
	Log, 
	Warning,
	Error,
	Fatal
};

//TD
char* ToString(int value);

class Debug
{
public:

	static void Log(std::string message, DebugLevel level = DebugLevel::Log, int LineNumber = -1, const char* funcName = nullptr);
};

