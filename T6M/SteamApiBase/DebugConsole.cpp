#include "STDInc.h"
#include "DebugConsole.h"

#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <string>

static CRITICAL_SECTION g_LogCS;
static bool g_LogReady = false;
static std::string g_LogBuffer;

static HANDLE g_ConsoleOut = NULL;

void DebugConsole::Initialize()
{
	if (g_LogReady)
		return;

	InitializeCriticalSection(&g_LogCS);

	AllocConsole();

	FILE* dummy = NULL;
	freopen_s(&dummy, "CONOUT$", "w", stdout);
	freopen_s(&dummy, "CONOUT$", "w", stderr);
	freopen_s(&dummy, "CONIN$", "r", stdin);

	g_ConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTitleA("T6M Debug Console");

	HWND hwnd = GetConsoleWindow();
	if (hwnd)
	{
		MoveWindow(hwnd, 80, 80, 950, 520, TRUE);
	}

	g_LogReady = true;

	WriteLine("INFO", "Console", "T6M debug console initialized");
	WriteLine("INFO", "Console", "Logs copy to clipboard on shutdown/crash");
}

void DebugConsole::Shutdown()
{
	if (!g_LogReady)
		return;

	WriteLine("INFO", "Console", "Shutdown requested, copying log to clipboard");

	CopyLogToClipboard();

	DeleteCriticalSection(&g_LogCS);

	g_LogReady = false;
}

void DebugConsole::Info(const char* category, const char* fmt, ...)
{
	char buffer[2048] = { 0 };

	va_list args;
	va_start(args, fmt);
	_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);
	va_end(args);

	WriteLine("INFO", category, buffer);
}

void DebugConsole::Warn(const char* category, const char* fmt, ...)
{
	char buffer[2048] = {};

	va_list args;

	va_start(args, fmt);

	vsnprintf_s(
		buffer,
		sizeof(buffer),
		_TRUNCATE,
		fmt,
		args);

	va_end(args);

	WriteLine("WARN", category, buffer);
}

void DebugConsole::Error(const char* category, const char* fmt, ...)
{
	char buffer[2048] = { 0 };

	va_list args;
	va_start(args, fmt);
	_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);
	va_end(args);

	WriteLine("ERROR", category, buffer);
}

void DebugConsole::EnterFunction(const char* functionName)
{
	if (!functionName)
		return;

	WriteLine("CALL", "Function", functionName);
}

void DebugConsole::LeaveFunction(const char* functionName)
{
	if (!functionName)
		return;

	WriteLine("DONE", "Function", functionName);
}

void DebugConsole::WriteLine(const char* level, const char* category, const char* text)
{
	if (!g_LogReady)
		return;

	EnterCriticalSection(&g_LogCS);

	SYSTEMTIME st;
	GetLocalTime(&st);

	char line[4096] = { 0 };

	_snprintf_s(
		line,
		sizeof(line),
		_TRUNCATE,
		"[%02d:%02d:%02d] %-5s %-12s %s\n",
		st.wHour,
		st.wMinute,
		st.wSecond,
		level ? level : "LOG",
		category ? category : "General",
		text ? text : "");

	AppendToBuffer(line);

	if (g_ConsoleOut)
	{
		if (strcmp(level, "ERROR") == 0)
			SetConsoleTextAttribute(g_ConsoleOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
		else if (strcmp(level, "WARN") == 0)
			SetConsoleTextAttribute(g_ConsoleOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		else if (strcmp(level, "CALL") == 0)
			SetConsoleTextAttribute(g_ConsoleOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		else
			SetConsoleTextAttribute(g_ConsoleOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

		printf("%s", line);

		SetConsoleTextAttribute(g_ConsoleOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}

	OutputDebugStringA(line);

	LeaveCriticalSection(&g_LogCS);
}

void DebugConsole::AppendToBuffer(const char* text)
{
	if (!text)
		return;

	g_LogBuffer += text;

	const size_t maxSize = 1024 * 256;

	if (g_LogBuffer.size() > maxSize)
	{
		g_LogBuffer.erase(0, g_LogBuffer.size() - maxSize);
	}
}

void DebugConsole::CopyLogToClipboard()
{
	if (g_LogBuffer.empty())
		return;

	if (!OpenClipboard(NULL))
		return;

	EmptyClipboard();

	size_t size = g_LogBuffer.size() + 1;

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
	if (!hMem)
	{
		CloseClipboard();
		return;
	}

	void* mem = GlobalLock(hMem);
	if (mem)
	{
		memcpy(mem, g_LogBuffer.c_str(), size);
		GlobalUnlock(hMem);

		SetClipboardData(CF_TEXT, hMem);
	}

	CloseClipboard();
}