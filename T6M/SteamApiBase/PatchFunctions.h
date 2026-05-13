#pragma once
#include "STDPatches.h"

void DisableMP44OnlineServiceChecks();
bool SafeWriteBytes(DWORD address, const BYTE* data, SIZE_T size, const char* name);
bool SafeWriteByte(DWORD address, BYTE value, const char* name);
bool SafeWriteDWORD(DWORD address, DWORD value, const char* name);
bool SafeReadDWORD(DWORD address, DWORD* value);
void SafeInitializeScriptability();


void forceOnlineGame_f();
void forceSystemLink_f();
void forceOfflineMode_f();
void forcePrivateMode_f();
void forceOffline_f();
void forceOnline_f();
void forceBoth_f();
void openConsole_f();
void testHudElems_f2();