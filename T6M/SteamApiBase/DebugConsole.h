#pragma once

#ifndef DEBUGCONSOLE_H
#define DEBUGCONSOLE_H

class DebugConsole
{
public:
	static void Initialize();
	static void Shutdown();

	static void Info(const char* category, const char* fmt, ...);
	static void Warn(const char* category, const char* fmt, ...);
	static void Error(const char* category, const char* fmt, ...);

	static void EnterFunction(const char* functionName);
	static void LeaveFunction(const char* functionName);

	static void WriteLine(const char* level, const char* category, const char* text);
	static void AppendToBuffer(const char* text);
	static void CopyLogToClipboard();
};

void EnableNativeTreyarchConsole();
void T6ConsoleLog(const char* category, const char* fmt, ...);

#endif