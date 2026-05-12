#pragma once

#include <Windows.h>
#include <string>

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

	static void CopyLogToClipboard();

private:
	static void WriteLine(const char* level, const char* category, const char* text);
	static void AppendToBuffer(const char* text);
};

#define LOG_INFO(cat, fmt, ...)  DebugConsole::Info(cat, fmt, __VA_ARGS__)
#define LOG_WARN(cat, fmt, ...)  DebugConsole::Warn(cat, fmt, __VA_ARGS__)
#define LOG_ERROR(cat, fmt, ...) DebugConsole::Error(cat, fmt, __VA_ARGS__)

#define LOG_SCOPE() FunctionScopeLogger __scope_logger(__FUNCTION__)

class FunctionScopeLogger
{
public:
	FunctionScopeLogger(const char* name)
	{
		functionName = name;
		DebugConsole::EnterFunction(functionName);
	}

	~FunctionScopeLogger()
	{
		DebugConsole::LeaveFunction(functionName);
	}

private:
	const char* functionName;
};