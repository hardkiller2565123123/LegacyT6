#pragma once

class InGameConsole
{
public:
	static void Initialize();
	static void Toggle();
	static void Log(const char* text);

	static bool IsOpen();
	static const char* DebugLog;
};