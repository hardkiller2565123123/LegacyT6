#define _CRT_SECURE_NO_WARNINGS

#include "STDInc.h"
#include "DebugConsole.h"
#include "STDPatches.h"
#include "PatchFunctions.h"
// ==========================================================
// State
// ==========================================================

static bool g_MP44LoggedDvarSuccess = false;
static bool g_MP44NativeConsoleStarted = false;

// ==========================================================
// Safe Memory Helpers
// ==========================================================

bool SafeWriteBytes(DWORD address, const BYTE* data, SIZE_T size, const char* name)
{
	if (!address || !data || !size)
	{
		if (name)
			MPLog(MP_COLOR_ERROR, "WRITE", "%s invalid args", name);

		return false;
	}

	__try
	{
		DWORD oldProtect = 0;

		if (!VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			if (name)
			{
				MPLog(
					MP_COLOR_ERROR,
					"WRITE",
					"%s VirtualProtect failed at 0x%08X err=%lu",
					name,
					address,
					GetLastError());
			}

			return false;
		}

		memcpy((void*)address, data, size);
		FlushInstructionCache(GetCurrentProcess(), (LPCVOID)address, size);

		DWORD tempProtect = 0;
		VirtualProtect((LPVOID)address, size, oldProtect, &tempProtect);

		if (name)
		{
			MPLog(
				MP_COLOR_OK,
				"WRITE",
				"%s OK -> 0x%08X size=%u",
				name,
				address,
				(unsigned int)size);
		}

		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (name)
		{
			MPLog(
				MP_COLOR_ERROR,
				"WRITE",
				"%s crashed -> 0x%08X size=%u",
				name,
				address,
				(unsigned int)size);
		}

		return false;
	}
}

bool SafeWriteByte(DWORD address, BYTE value, const char* name)
{
	return SafeWriteBytes(address, &value, sizeof(value), name);
}

bool SafeWriteDWORD(DWORD address, DWORD value, const char* name)
{
	return SafeWriteBytes(address, (const BYTE*)&value, sizeof(value), name);
}

bool SafeReadDWORD(DWORD address, DWORD* out)
{
	if (!address || !out)
		return false;

	__try
	{
		*out = *(DWORD*)address;
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		*out = 0;
		return false;
	}
}

bool SafePatchReturnTrue(DWORD address, const char* name)
{
	BYTE patch[] =
	{
		0xB8, 0x01, 0x00, 0x00, 0x00,
		0xC3
	};

	return SafeWriteBytes(address, patch, sizeof(patch), name);
}

bool SafePatchReturn(DWORD address, const char* name)
{
	BYTE patch = 0xC3;
	return SafeWriteByte(address, patch, name);
}

bool SafeNop(DWORD address, SIZE_T size, const char* name)
{
	BYTE buffer[64] = { 0 };

	if (size > sizeof(buffer))
		size = sizeof(buffer);

	memset(buffer, 0x90, size);

	return SafeWriteBytes(address, buffer, size, name);
}

// ==========================================================
// Safe Engine Helpers
// ==========================================================

void SafeAddCommand(const char* name, void(__cdecl* function)(), cmd_function_s* command)
{
	if (!Addresses::Cmd_AddCommandInternal || !name || !function || !command)
	{
		MPLog(MP_COLOR_WARN, "CMD", "SafeAddCommand skipped: %s", name ? name : "NULL");
		return;
	}

	__try
	{
		Addresses::Cmd_AddCommandInternal(name, function, command);
		MPLog(MP_COLOR_OK, "CMD", "registered: %s", name);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MPLog(MP_COLOR_ERROR, "CMD", "Cmd_AddCommandInternal crashed: %s", name);
	}
}

void SafeStartConsoleThread(LPVOID startupAddress)
{
	if (!startupAddress)
		return;

	__try
	{
		HANDLE thread = CreateThread(0, 0, ConsoleWindow, startupAddress, 0, 0);

		if (thread)
			CloseHandle(thread);

		MPLog(MP_COLOR_OK, "THREAD", "game console thread started -> 0x%08X", startupAddress);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MPLog(MP_COLOR_ERROR, "THREAD", "ConsoleWindow thread crashed");
	}
}

void SafeCbufAddText(const char* text)
{
	if (!text || !text[0])
		return;

	if (!Addresses::Cbuf_AddText)
	{
		MPLog(MP_COLOR_WARN, "CBUF", "Cbuf_AddText missing: %s", text);
		return;
	}

	__try
	{
		Addresses::Cbuf_AddText(0, text);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MPLog(MP_COLOR_ERROR, "CBUF", "Cbuf_AddText crashed: %s", text);
	}
}

typedef void(__cdecl* SessionModeFunc_t)();

static void SafeCallSessionMode(DWORD address, const char* name)
{
	if (!address)
	{
		MPLog(MP_COLOR_WARN, "LUA", "%s address is NULL", name ? name : "SessionMode");
		return;
	}

	__try
	{
		SessionModeFunc_t fn = (SessionModeFunc_t)address;
		fn();

		MPLog(MP_COLOR_OK, "LUA", "%s finished", name ? name : "SessionMode");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MPLog(MP_COLOR_ERROR, "LUA", "%s crashed", name ? name : "SessionMode");
	}
}

// ==========================================================
// Dvar Helpers
// ==========================================================

static bool SafeSetDvarBoolByPointer(DWORD pointerAddress, bool enabled, const char* name, bool quiet)
{
	DWORD dvar = 0;

	if (!SafeReadDWORD(pointerAddress, &dvar) || !dvar)
	{
		if (!quiet)
			MPLog(MP_COLOR_WARN, "DVAR", "%s pointer not ready", name ? name : "unknown");

		return false;
	}

	BYTE value = enabled ? 1 : 0;

	bool ok = true;
	ok &= SafeWriteBytes(dvar + 0x18, &value, 1, quiet ? NULL : name);
	ok &= SafeWriteBytes(dvar + 0x1C, &value, 1, quiet ? NULL : name);

	return ok;
}

static bool ForceMPAllFrontendOnce(bool quiet)
{
	bool ok = true;

	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_DEVELOPER, true, "[MP V44] developer", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_DEVELOPER_SCRIPT, true, "[MP V44] developer_script", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_LUI_CHECKSUM_ENABLED, true, "[MP V44] lui_checksum_enabled", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_XBLIVE_RANKEDMATCH, true, "[MP V44] xblive_rankedmatch", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_ONLINEGAME, true, "[MP V44] onlinegame", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_SYSTEMLINK, true, "[MP V44] systemlink", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_XBLIVE_PRIVATEMATCH, true, "[MP V44] xblive_privatematch", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_SV_DISABLECLIENTCONSOLE, false, "[MP V44] sv_disableClientConsole", quiet);

	if (ok && !g_MP44LoggedDvarSuccess)
	{
		MPLog(MP_COLOR_OK, "MPV44", "all frontend dvars forced OK");
		g_MP44LoggedDvarSuccess = true;
	}
	else if (!ok && !quiet)
	{
		MPLog(MP_COLOR_WARN, "MPV44", "frontend dvars not ready");
	}

	return ok;
}

static void StartNativeConsoleOnce()
{
	if (g_MP44NativeConsoleStarted)
		return;

	g_MP44NativeConsoleStarted = true;

	EnableNativeTreyarchConsole();
}

// ==========================================================
// MP V44 Online Service Bypass
// ==========================================================

void DisableMP44OnlineServiceChecks()
{
	MPLog(MP_COLOR_INFO, "MPV44", "online service patch skipped for stability");

	// Leave this disabled until the crash is gone.
	// SafePatchReturn(
	//     MP44_FUNC_POPUP_CONNECTINGDW,
	//     "[MP V44] disable popup_connectingdw");
}

void DisableMP44OnlineServicePopupCalls()
{
	MPLog(MP_COLOR_INFO, "MPV44", "disabling online service popup calls only");

	SafeNop(MP44_CALL_POPUP_GETTINGDATA, 10, "[MP V44] nop popup_gettingdata");
	SafeNop(MP44_CALL_POPUP_CONNECTINGTODW, 10, "[MP V44] nop popup_connectingtodw");
}

// ==========================================================
// Frontend Thread
// ==========================================================

DWORD WINAPI MP44FrontendWaitThread(LPVOID)
{
	__try
	{
		MPLog(MP_COLOR_INFO, "BOOT", "Frontend thread started");

		// Let BO2 finish frontend startup first.
		Sleep(5000);

		MPLog(MP_COLOR_INFO, "MPV44", "waiting for frontend dvars");

		bool found = false;

		for (int i = 0; i < 300; i++)
		{
			DWORD dev = 0;
			DWORD online = 0;
			DWORD systemlink = 0;

			SafeReadDWORD(MP44_DVAR_DEVELOPER_SCRIPT, &dev);
			SafeReadDWORD(MP44_DVAR_ONLINEGAME, &online);
			SafeReadDWORD(MP44_DVAR_SYSTEMLINK, &systemlink);

			if (dev || online || systemlink)
			{
				found = true;

				MPLog(
					MP_COLOR_INFO,
					"MPV44",
					"frontend dvars ready");

				break;
			}

			Sleep(100);
		}

		if (!found)
		{
			MPLog(MP_COLOR_WARN, "MPV44", "frontend dvars not ready, skipping forced writes");
			StartNativeConsoleOnce();
			return 0;
		}

		// Start native console FIRST.
		StartNativeConsoleOnce();

		// Delay before touching dvars.
		Sleep(3000);

		// Force only once for now.
		ForceMPAllFrontendOnce(false);

		MPLog(MP_COLOR_OK, "MPV44", "frontend thread finished");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MessageBoxA(
			NULL,
			"MP44FrontendWaitThread crashed",
			"T6",
			MB_ICONERROR);
	}

	return 0;
}

// ==========================================================
// Scriptability
// ==========================================================

void SafeInitializeScriptability()
{
	MPLog(MP_COLOR_INFO, "SCRIPT", "initializing scriptability hooks");

	__try
	{
		Script::Initialize();
		MPLog(MP_COLOR_OK, "SCRIPT", "scriptability initialized");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MPLog(MP_COLOR_ERROR, "SCRIPT", "scriptability crashed");
	}
}

// ==========================================================
// Manual Commands
// ==========================================================

void forceOnlineGame_f()
{
	SafeCallSessionMode(0x00971260, "SessionModeSetOnlineGame");
}

void forceSystemLink_f()
{
	SafeCallSessionMode(0x009712A0, "SessionModeSetSystemLink");
}

void forceOfflineMode_f()
{
	SafeCallSessionMode(0x009712E0, "SessionModeSetOffline");
}

void forcePrivateMode_f()
{
	SafeCallSessionMode(0x00971320, "SessionModeSetPrivate");
}

void forceOffline_f()
{
	MPLog(MP_COLOR_INFO, "LAN", "forcing LAN/System Link frontend state");

	ForceMPAllFrontendOnce(false);

	SafeCbufAddText("set developer 1\n");
	SafeCbufAddText("set developer_script 1\n");
	SafeCbufAddText("set lui_checksum_enabled 1\n");
	SafeCbufAddText("set onlinegame 1\n");
	SafeCbufAddText("set systemlink 1\n");
	SafeCbufAddText("set ui_netSource 1\n");
	SafeCbufAddText("set xblive_privatematch 1\n");
	SafeCbufAddText("set xblive_rankedmatch 1\n");
	SafeCbufAddText("set xblive_loggedin 1\n");
	SafeCbufAddText("set sv_disableClientConsole 0\n");
	SafeCbufAddText("openmenu menu_systemlink_lobby\n");
}

void forceOnline_f()
{
	MPLog(MP_COLOR_INFO, "ONLINE", "forcing Online frontend state");

	ForceMPAllFrontendOnce(false);

	SafeCbufAddText("set developer 1\n");
	SafeCbufAddText("set developer_script 1\n");
	SafeCbufAddText("set lui_checksum_enabled 1\n");
	SafeCbufAddText("set onlinegame 1\n");
	SafeCbufAddText("set systemlink 1\n");
	SafeCbufAddText("set ui_netSource 2\n");
	SafeCbufAddText("set xblive_privatematch 1\n");
	SafeCbufAddText("set xblive_rankedmatch 1\n");
	SafeCbufAddText("set xblive_loggedin 1\n");
	SafeCbufAddText("set sv_disableClientConsole 0\n");
	SafeCbufAddText("openmenu menu_xboxlive_lobby\n");
}

void forceBoth_f()
{
	MPLog(MP_COLOR_INFO, "BOTH", "forcing LAN + Online frontend state");

	ForceMPAllFrontendOnce(false);

	SafeCbufAddText("set developer 1\n");
	SafeCbufAddText("set developer_script 1\n");
	SafeCbufAddText("set lui_checksum_enabled 1\n");
	SafeCbufAddText("set onlinegame 1\n");
	SafeCbufAddText("set systemlink 1\n");
	SafeCbufAddText("set xblive_privatematch 1\n");
	SafeCbufAddText("set xblive_rankedmatch 1\n");
	SafeCbufAddText("set xblive_loggedin 1\n");
	SafeCbufAddText("set sv_disableClientConsole 0\n");
}

void openConsole_f()
{
	MPLog(MP_COLOR_INFO, "CONSOLE", "forcing console dvars");

	ForceMPAllFrontendOnce(false);
	StartNativeConsoleOnce();

	SafeCbufAddText("set sv_disableClientConsole 0\n");
	SafeCbufAddText("set con_minicon 1\n");
	SafeCbufAddText("set developer 1\n");
	SafeCbufAddText("set developer_script 1\n");
	SafeCbufAddText("set monkeytoy 0\n");


}

// ==========================================================
// HUD Test
// ==========================================================

void spawnTestElem(int client)
{
	__try
	{
		HudElem* elem = HudElements::HudElem_Allocate();

		if (!elem)
			return;

		HudElements::HudElem_Init(elem, client);
		HudElements::HudElem_SetText(elem, "Sponsored!");
		HudElements::HudElem_SetAlign(elem, TOP_CENTER);
		HudElements::HudElem_SetColor(elem, 0, 0, 0, 255);
		HudElements::HudElem_SetGlow(elem, 255, 0, 0, 80);

		elem->fontScale = 2.0f;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MPLog(MP_COLOR_ERROR, "HUD", "spawnTestElem crashed");
	}
}

void testHudElems_f2()
{
	if (!Addresses::SV_Loaded)
		return;

	if (!Addresses::SV_Loaded())
		return;

	for (int i = 0; i < 4; i++)
		spawnTestElem(i);
}

// ==========================================================
// Old External Command Thread
// Kept for linker compatibility only.
// Do not start this if using native Treyarch console.
// ==========================================================

DWORD WINAPI V44CommandThread(LPVOID)
{
	MPLog(MP_COLOR_WARN, "CMD", "old external command thread started");
	MPLog(MP_COLOR_WARN, "CMD", "native Treyarch console should be used instead");

	char input[256] = { 0 };

	while (true)
	{
		if (!fgets(input, sizeof(input), stdin))
		{
			Sleep(100);
			continue;
		}

		input[strcspn(input, "\r\n")] = 0;

		if (_stricmp(input, "help") == 0)
		{
			MPLog(MP_COLOR_INFO, "HELP", "commands: help, lan, online, both, openConsole, initOverlay, quit");
		}
		else if (_stricmp(input, "lan") == 0 || _stricmp(input, "forceOffline") == 0)
		{
			forceOffline_f();
		}
		else if (_stricmp(input, "online") == 0 || _stricmp(input, "forceOnline") == 0)
		{
			forceOnline_f();
		}
		else if (_stricmp(input, "both") == 0)
		{
			forceBoth_f();
		}
		else if (_stricmp(input, "openConsole") == 0)
		{
			openConsole_f();
		}
		else if (_stricmp(input, "initOverlay") == 0)
		{
			__try
			{
				InGameConsole::Initialize();
				MPLog(MP_COLOR_OK, "OVERLAY", "initialized");
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				MPLog(MP_COLOR_ERROR, "OVERLAY", "init crashed");
			}
		}
		else if (_stricmp(input, "quit") == 0)
		{
			break;
		}
		else if (input[0])
		{
			SafeCbufAddText(input);
			SafeCbufAddText("\n");
		}

		memset(input, 0, sizeof(input));
		Sleep(10);
	}

	return 0;
}

