#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <vector>
#include <map>

#include "STDInc.h"
#include "Define.h"
#include "DebugConsole.h"
#include "PatchT6MP.h"
#include "CEG.h"

enum MPLogColor
{
	MP_COLOR_DEFAULT = 7,
	MP_COLOR_INFO = 11,
	MP_COLOR_OK = 10,
	MP_COLOR_WARN = 14,
	MP_COLOR_ERROR = 12
};

void MPLog(MPLogColor color, const char* label, const char* fmt, ...);

#define LOG_SCOPE() DebugConsole::EnterFunction(__FUNCTION__)

bool SafeWriteBytes(DWORD address, const BYTE* data, SIZE_T size, const char* name);
bool SafeWriteByte(DWORD address, BYTE value, const char* name);
bool SafeWriteDWORD(DWORD address, DWORD value, const char* name);
bool SafeReadDWORD(DWORD address, DWORD* out);
bool SafePatchReturnTrue(DWORD address, const char* name);
bool SafePatchReturn(DWORD address, const char* name);
bool SafeNop(DWORD address, SIZE_T size, const char* name);

void SafeAddCommand(const char* name, void(__cdecl* function)(), cmd_function_s* command);
void SafeStartConsoleThread(LPVOID startupAddress);
void SafeCbufAddText(const char* text);
void SafeInitializeScriptability();

void EnableNativeTreyarchConsole();
void DisableMP44OnlineServiceChecks();
void DisableMP44OnlineServicePopupCalls();

DWORD WINAPI MP44FrontendWaitThread(LPVOID);
DWORD WINAPI V44CommandThread(LPVOID);

void forceOnlineGame_f();
void forceSystemLink_f();
void forceOfflineMode_f();
void forcePrivateMode_f();
void forceOffline_f();
void forceOnline_f();
void forceBoth_f();
void openConsole_f();

void spawnTestElem(int client);
void testHudElems_f2();