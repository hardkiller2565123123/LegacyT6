#define _CRT_SECURE_NO_WARNINGS

#include "STDInc.h"
#include "DebugConsole.h"
#include "STDPatches.h"
#include "PatchFunctions.h"

#include <stdio.h>
#include <stdarg.h>
#include <string>

#ifndef MP44_FUNC_WINCONSOLE_INIT
#define MP44_FUNC_WINCONSOLE_INIT 0x005E38C0
#endif

typedef int(__cdecl* WinConsoleInit_t)(HINSTANCE hInstance);

static CRITICAL_SECTION g_LogCS;
static bool g_LogReady = false;
static std::string g_LogBuffer;

static HWND g_TreyarchConsoleWindow = NULL;
static HWND g_TreyarchConsoleOutput = NULL;
static HWND g_TreyarchConsoleInput = NULL;

static WNDPROC g_TreyarchInputOldProc = NULL;
static WNDPROC g_TreyarchWindowOldProc = NULL;

static bool g_T6VerboseNativeConsole = false;
static bool g_NativeConsoleStarted = false;

static void RefreshTreyarchConsoleControls();
static void T6ConsoleClear();

void DebugConsole::Initialize()
{
	if (g_LogReady)
		return;

	InitializeCriticalSection(&g_LogCS);
	g_LogReady = true;

	OutputDebugStringA("[T6] DebugConsole initialized\n");
}

void DebugConsole::Shutdown()
{
	if (!g_LogReady)
		return;

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
	char buffer[2048] = { 0 };

	va_list args;
	va_start(args, fmt);
	_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);
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
	if (functionName)
		WriteLine("CALL", "Function", functionName);
}

void DebugConsole::LeaveFunction(const char* functionName)
{
	if (functionName)
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
		g_LogBuffer.erase(0, g_LogBuffer.size() - maxSize);
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

static bool ShouldMirrorToNativeConsole(const char* label)
{
	if (!label)
		return true;

	if (g_T6VerboseNativeConsole)
		return true;

	if (_stricmp(label, "WRITE") == 0)
		return false;

	if (_stricmp(label, "SKIP") == 0)
		return false;

	return true;
}



static BOOL CALLBACK FindConsoleChildProc(HWND hwnd, LPARAM)
{
	char className[64] = { 0 };
	GetClassNameA(hwnd, className, sizeof(className));

	if (_stricmp(className, "Edit") != 0)
		return TRUE;

	LONG id = GetWindowLongA(hwnd, GWL_ID);

	if (id == 0x64)
		g_TreyarchConsoleOutput = hwnd;
	else if (id == 0x65)
		g_TreyarchConsoleInput = hwnd;

	return TRUE;
}

static void RefreshTreyarchConsoleControls()
{
	g_TreyarchConsoleWindow = FindWindowA(
		"CoD Black Ops II WinConsole",
		"CoD Black Ops II Console");

	g_TreyarchConsoleOutput = NULL;
	g_TreyarchConsoleInput = NULL;

	if (g_TreyarchConsoleWindow)
		EnumChildWindows(g_TreyarchConsoleWindow, FindConsoleChildProc, 0);
}

static void T6ConsolePrintRaw(const char* text)
{
	if (!text)
		return;

	if (!g_TreyarchConsoleOutput)
		RefreshTreyarchConsoleControls();

	if (!g_TreyarchConsoleOutput)
		return;

	int len = GetWindowTextLengthA(g_TreyarchConsoleOutput);

	SendMessageA(g_TreyarchConsoleOutput, EM_SETSEL, len, len);
	SendMessageA(g_TreyarchConsoleOutput, EM_REPLACESEL, FALSE, (LPARAM)text);
}

static void T6ConsoleClear()
{
	if (!g_TreyarchConsoleOutput)
		RefreshTreyarchConsoleControls();

	if (g_TreyarchConsoleOutput)
		SetWindowTextA(g_TreyarchConsoleOutput, "");
}

void T6ConsoleLog(const char* category, const char* fmt, ...)
{
	if (!fmt)
		return;

	static CRITICAL_SECTION cs;
	static bool initialized = false;
	static bool recursionGuard = false;

	if (!initialized)
	{
		InitializeCriticalSection(&cs);
		initialized = true;
	}

	if (recursionGuard)
		return;

	EnterCriticalSection(&cs);

	recursionGuard = true;

	if (!g_TreyarchConsoleOutput)
		RefreshTreyarchConsoleControls();

	if (g_TreyarchConsoleOutput)
	{
		char message[2048] = { 0 };
		char finalMessage[2300] = { 0 };

		va_list ap;
		va_start(ap, fmt);
		_vsnprintf_s(message, sizeof(message), _TRUNCATE, fmt, ap);
		va_end(ap);

		sprintf_s(
			finalMessage,
			sizeof(finalMessage),
			"[T6] %-10s %s\r\n",
			category ? category : "LOG",
			message);

		T6ConsolePrintRaw(finalMessage);
	}

	recursionGuard = false;

	LeaveCriticalSection(&cs);
}

void MPLog(MPLogColor color, const char* label, const char* fmt, ...)
{
	char message[2048] = { 0 };
	char finalLine[4096] = { 0 };

	va_list args;
	va_start(args, fmt);
	_vsnprintf_s(message, sizeof(message), _TRUNCATE, fmt, args);
	va_end(args);

	sprintf_s(
		finalLine,
		sizeof(finalLine),
		"[%-7s] %s\n",
		label ? label : "MP",
		message);

	OutputDebugStringA(finalLine);

	T6ConsoleLog(label ? label : "MP", "%s", message);
}

static void RunNativeConsoleCommand(const char* command)
{
	if (!command || !command[0])
		return;

	T6ConsoleLog("CMD", "%s", command);

	if (_stricmp(command, "help") == 0)
	{
		T6ConsoleLog("HELP", "help, clear, verbose, quiet, lan, online, both, quit");
		return;
	}

	if (_stricmp(command, "clear") == 0)
	{
		T6ConsoleClear();
		return;
	}

	if (_stricmp(command, "verbose") == 0)
	{
		g_T6VerboseNativeConsole = true;
		T6ConsoleLog("MODE", "verbose enabled");
		return;
	}

	if (_stricmp(command, "quiet") == 0)
	{
		g_T6VerboseNativeConsole = false;
		T6ConsoleLog("MODE", "verbose disabled");
		return;
	}

	if (_stricmp(command, "lan") == 0)
	{
		forceOffline_f();
		T6ConsoleLog("MODE", "LAN/System Link dvars forced");
		return;
	}

	if (_stricmp(command, "online") == 0)
	{
		forceOnline_f();
		T6ConsoleLog("MODE", "Online dvars forced");
		return;
	}

	if (_stricmp(command, "both") == 0)
	{
		forceBoth_f();
		T6ConsoleLog("MODE", "Online + System Link dvars forced");
		return;
	}

	if (_stricmp(command, "quit") == 0 || _stricmp(command, "exit") == 0)
	{
		T6ConsoleLog("QUIT", "closing game");
		ExitProcess(0);
		return;
	}

	T6ConsoleLog("BLOCK", "engine command blocked for stability: %s", command);
}

static LRESULT CALLBACK TreyarchInputProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_KEYDOWN && wParam == VK_RETURN)
	{
		char command[512] = { 0 };

		GetWindowTextA(hwnd, command, sizeof(command));
		SetWindowTextA(hwnd, "");

		RunNativeConsoleCommand(command);

		return 0;
	}

	if (g_TreyarchInputOldProc)
		return CallWindowProcA(g_TreyarchInputOldProc, hwnd, msg, wParam, lParam);

	return DefWindowProcA(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK TreyarchWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CLOSE || msg == WM_DESTROY)
	{
		T6ConsoleLog("QUIT", "native console closed, exiting BO2");
		ExitProcess(0);
		return 0;
	}

	if (g_TreyarchWindowOldProc)
		return CallWindowProcA(g_TreyarchWindowOldProc, hwnd, msg, wParam, lParam);

	return DefWindowProcA(hwnd, msg, wParam, lParam);
}

static void HookTreyarchConsoleInput()
{
	RefreshTreyarchConsoleControls();

	if (!g_TreyarchConsoleInput)
		return;

	if (g_TreyarchInputOldProc)
		return;

	__try
	{
		g_TreyarchInputOldProc =
			(WNDPROC)SetWindowLongPtrA(
				g_TreyarchConsoleInput,
				GWLP_WNDPROC,
				(LONG_PTR)TreyarchInputProc);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		g_TreyarchInputOldProc = NULL;
	}
}

static void HookTreyarchConsoleWindow()
{
	RefreshTreyarchConsoleControls();

	if (!g_TreyarchConsoleWindow)
		return;

	if (g_TreyarchWindowOldProc)
		return;

	__try
	{
		g_TreyarchWindowOldProc =
			(WNDPROC)SetWindowLongPtrA(
				g_TreyarchConsoleWindow,
				GWLP_WNDPROC,
				(LONG_PTR)TreyarchWindowProc);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		g_TreyarchWindowOldProc = NULL;
	}
}

static void ActivateTreyarchConsoleWindow()
{
	RefreshTreyarchConsoleControls();

	if (!g_TreyarchConsoleWindow)
		return;

	ShowWindow(g_TreyarchConsoleWindow, SW_SHOW);
	BringWindowToTop(g_TreyarchConsoleWindow);
}

static DWORD WINAPI TreyarchConsoleThread(LPVOID)
{
	__try
	{
		HINSTANCE hInstance = GetModuleHandleA(NULL);

		((WinConsoleInit_t)MP44_FUNC_WINCONSOLE_INIT)(hInstance);

		Sleep(2000);

		ActivateTreyarchConsoleWindow();
		HookTreyarchConsoleWindow();
		HookTreyarchConsoleInput();

		T6ConsoleClear();

		T6ConsoleLog("BOOT", "PlusOps T6MP V44");
		T6ConsoleLog("BOOT", "Native Treyarch console ready");
		T6ConsoleLog("MODE", "Online + System Link enabled");
		T6ConsoleLog("HELP", "Type help for commands");

		MSG msg;

		while (true)
		{
			while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					return 0;

				TranslateMessage(&msg);
				DispatchMessageA(&msg);
			}

			Sleep(10);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		OutputDebugStringA("[T6] Treyarch console thread crashed\n");
	}

	return 0;
}

void EnableNativeTreyarchConsole()
{
	if (g_NativeConsoleStarted)
		return;

	g_NativeConsoleStarted = true;

	HANDLE thread = CreateThread(
		NULL,
		0,
		TreyarchConsoleThread,
		NULL,
		0,
		NULL);

	if (thread)
		CloseHandle(thread);
}