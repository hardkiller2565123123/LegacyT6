#include "STDInc.h"
#include "../../../PlusOpsSource/PlusOpsSource/SteamApiBase/CEG.h"
#include "DebugConsole.h"
#include "PatchT6MP.h"

// ==========================================================
// MP V44 Strings / Notes
// ==========================================================

#define V44_MENU_OFFLINE_STRING         0x00BFF3D8 // MENU_OFFLINE
#define V44_MENU_SYSTEMLINK_LOBBY       0x00BD15E8 // menu_systemlink_lobby
#define V44_MENU_SYSTEMLINK_STRING2     0x00C2C1C8 // menu_systemlink...
#define V44_SYSTEMLINK_STRING           0x00C2E428 // systemlink
#define V44_ONLINEGAME_STRING           0x00C5F25C // onlinegame
#define V44_OFFLINEHOST_STRING          0x00C0BD9C // offlinehost
#define V44_XBLIVE_LOGGEDIN_STRING      0x00C27B20 // xblive_loggedin
#define V44_XBLIVE_PRIVATEMATCH_STRING  0x00C0EB30 // xblive_privatematch

// ==========================================================
// MP V44 DVAR POINTER GLOBALS
// Found from sub_88B9C0
// ==========================================================

#define MP44_DVAR_LUI_CHECKSUM_ENABLED   0x025E3CF8
#define MP44_DVAR_ONLINEGAME             0x02562AF4
#define MP44_DVAR_XBLIVE_RANKEDMATCH     0x025E2CD8
#define MP44_DVAR_XBLIVE_PRIVATEMATCH    0x02562B44
#define MP44_DVAR_DEVELOPER              0x02562AEC
#define MP44_DVAR_DEVELOPER_SCRIPT       0x02562AD0

// Found near "systemlink" registration.
#define MP44_DVAR_SYSTEMLINK             0x011C7824

// ==========================================================
// Logging
// ==========================================================

enum MPLogColor
{
	MP_COLOR_DEFAULT = 7,
	MP_COLOR_INFO = 11,
	MP_COLOR_OK = 10,
	MP_COLOR_WARN = 14,
	MP_COLOR_ERROR = 12
};

static bool g_MP44LoggedDvarSuccess = false;
static char g_LastMPLogLine[768] = { 0 };
static int g_LastMPLogRepeat = 0;

static void MPSetConsoleColor(MPLogColor color)
{
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	if (console && console != INVALID_HANDLE_VALUE)
		SetConsoleTextAttribute(console, (WORD)color);
}

static void MPLog(MPLogColor color, const char* label, const char* fmt, ...)
{
	char message[512] = { 0 };
	char finalLine[768] = { 0 };

	va_list args;
	va_start(args, fmt);
	_vsnprintf_s(message, sizeof(message), _TRUNCATE, fmt, args);
	va_end(args);

	sprintf_s(finalLine, sizeof(finalLine), "[%-6s] %s", label ? label : "MP", message);

	if (_stricmp(g_LastMPLogLine, finalLine) == 0)
	{
		g_LastMPLogRepeat++;
		return;
	}

	if (g_LastMPLogRepeat > 0)
	{
		MPSetConsoleColor(MP_COLOR_WARN);
		printf("[SKIP  ] previous line repeated %d times\n", g_LastMPLogRepeat);
		MPSetConsoleColor(MP_COLOR_DEFAULT);
		g_LastMPLogRepeat = 0;
	}

	strcpy_s(g_LastMPLogLine, sizeof(g_LastMPLogLine), finalLine);

	MPSetConsoleColor(color);
	printf("%s\n", finalLine);
	MPSetConsoleColor(MP_COLOR_DEFAULT);
}

static void DebugLine(const char* text)
{
	if (!text)
		return;

	OutputDebugStringA(text);
	OutputDebugStringA("\n");
}

// ==========================================================
// Safe Memory Helpers
// ==========================================================

static bool SafeWriteBytes(DWORD address, const BYTE* data, SIZE_T size, const char* name)
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

		DWORD temp = 0;
		VirtualProtect((LPVOID)address, size, oldProtect, &temp);

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

static bool SafeWriteByte(DWORD address, BYTE value, const char* name)
{
	return SafeWriteBytes(address, &value, 1, name);
}

static bool SafeWriteDWORD(DWORD address, DWORD value, const char* name)
{
	return SafeWriteBytes(address, (const BYTE*)&value, sizeof(value), name);
}

static bool SafeReadDWORD(DWORD address, DWORD* out)
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

static bool SafePatchShortJump(DWORD address, const char* name)
{
	BYTE patch = 0xEB;
	return SafeWriteByte(address, patch, name);
}

static bool SafeNop(DWORD address, SIZE_T size, const char* name)
{
	BYTE buffer[64] = { 0 };

	if (size > sizeof(buffer))
		size = sizeof(buffer);

	memset(buffer, 0x90, size);

	return SafeWriteBytes(address, buffer, size, name);
}

// ==========================================================
// Safe Engine / Command Helpers
// ==========================================================

static void SafeAddCommand(
	const char* name,
	void(__cdecl* function)(),
	cmd_function_s* command)
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

static void SafeStartConsoleThread(LPVOID startupAddress)
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

static void SafeCbufAddText(const char* text)
{
	if (!Addresses::Cbuf_AddText || !text)
	{
		MPLog(MP_COLOR_WARN, "CBUF", "Cbuf_AddText missing");
		return;
	}

	__try
	{
		MPLog(MP_COLOR_INFO, "CBUF", "%s", text);
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
		MPLog(MP_COLOR_INFO, "LUA", "calling %s at 0x%08X", name ? name : "SessionMode", address);

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
// MP V44 DVAR Forcing
// ==========================================================

static bool SafeSetDvarBoolByPointer(DWORD pointerAddress, bool enabled, const char* name, bool quiet)
{
	DWORD dvar = 0;

	if (!SafeReadDWORD(pointerAddress, &dvar) || !dvar)
	{
		if (!quiet)
			MPLog(MP_COLOR_WARN, "DVAR", "%s pointer is NULL", name ? name : "unknown");

		return false;
	}

	BYTE value = enabled ? 1 : 0;

	SafeWriteBytes(dvar + 0x18, &value, 1, quiet ? NULL : name);
	SafeWriteBytes(dvar + 0x1C, &value, 1, quiet ? NULL : name);

	return true;
}

static bool ForceMPSystemLinkOnce(bool quiet)
{
	bool ok = true;

	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_DEVELOPER, true, "[MP V44] developer", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_DEVELOPER_SCRIPT, true, "[MP V44] developer_script", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_LUI_CHECKSUM_ENABLED, true, "[MP V44] lui_checksum_enabled", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_XBLIVE_RANKEDMATCH, true, "[MP V44] xblive_rankedmatch", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_ONLINEGAME, false, "[MP V44] onlinegame", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_XBLIVE_PRIVATEMATCH, true, "[MP V44] xblive_privatematch", quiet);
	ok &= SafeSetDvarBoolByPointer(MP44_DVAR_SYSTEMLINK, true, "[MP V44] systemlink", quiet);

	if (ok && !g_MP44LoggedDvarSuccess)
	{
		MPLog(MP_COLOR_OK, "MPV44", "systemlink/frontend dvars forced OK");
		g_MP44LoggedDvarSuccess = true;
	}
	else if (!ok && !quiet)
	{
		MPLog(MP_COLOR_WARN, "MPV44", "systemlink/frontend dvars not ready");
	}

	return ok;
}

static DWORD WINAPI MP44FrontendWaitThread(LPVOID)
{
	MPLog(MP_COLOR_WARN, "MPV44", "waiting for frontend dvars...");

	for (int i = 0; i < 600; i++)
	{
		DWORD dev = 0;
		DWORD online = 0;
		DWORD systemlink = 0;

		SafeReadDWORD(MP44_DVAR_DEVELOPER_SCRIPT, &dev);
		SafeReadDWORD(MP44_DVAR_ONLINEGAME, &online);
		SafeReadDWORD(MP44_DVAR_SYSTEMLINK, &systemlink);

		if (dev || online || systemlink)
		{
			MPLog(
				MP_COLOR_INFO,
				"MPV44",
				"frontend dvars found dev=0x%08X online=0x%08X systemlink=0x%08X",
				dev,
				online,
				systemlink);

			break;
		}

		Sleep(100);
	}

	for (int i = 0; i < 60; i++)
	{
		ForceMPSystemLinkOnce(i != 0);
		Sleep(500);
	}

	MPLog(MP_COLOR_INFO, "MPV44", "frontend dvar forcing finished");
	return 0;
}

// ==========================================================
// Manual Command Functions
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
	MPLog(MP_COLOR_INFO, "MP", "forcing offline / LAN menu state");

	ForceMPSystemLinkOnce(false);

	SafeCbufAddText("set onlinegame 0\n");
	SafeCbufAddText("set systemlink 1\n");
	SafeCbufAddText("set ui_netSource 1\n");
	SafeCbufAddText("set xblive_privatematch 1\n");
	SafeCbufAddText("set xblive_loggedin 1\n");
	SafeCbufAddText("disconnect\n");
	SafeCbufAddText("openmenu menu_systemlink_lobby\n");
}

void openConsole_f()
{
	MPLog(MP_COLOR_INFO, "MP", "forcing console dvars + toggleconsole");

	SafeCbufAddText("set developer 1\n");
	SafeCbufAddText("set monkeytoy 0\n");
	SafeCbufAddText("set con_minicon 1\n");
	SafeCbufAddText("toggleconsole\n");
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

		HudElem* material = HudElements::HudElem_Allocate();

		if (!material)
			return;

		HudElements::HudElem_Init(material, client);
		HudElements::HudElem_SetIcon(material, "hud_medals_nuclear");
		HudElements::HudElem_SetAlign(material, TOP_CENTER);

		material->width = 60;
		material->height = 60;
		material->y = 30;
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
	{
		spawnTestElem(i);
	}
}

// ==========================================================
// V40 Multiplayer
// ==========================================================

void T6MP::PatchT6MP_V40()
{
	MPLog(MP_COLOR_INFO, "V40", "patch start");

	static cmd_function_s testHudElems_cmd;
	static cmd_function_s autoChangeClass_cmd;
	static cmd_function_s Notify_cmd;
	static cmd_function_s spawnBot_cmd;

	SafeAddCommand("testHudElems", testHudElems_f2, &testHudElems_cmd);
	SafeAddCommand("autoChangeClass", Bots::autoChangeClass_f, &autoChangeClass_cmd);
	SafeAddCommand("Notify", PopupNotify::Notify_f, &Notify_cmd);
	SafeAddCommand("spawnBot", Bots::spawnBot_f, &spawnBot_cmd);

	SafeWriteByte(0x5BFA6B, 0xEB, "[V40] fix jmp");
	SafeWriteByte(0x5DDC20, 0xC3, "[V40] unknown patch");
	SafeWriteByte(0x88A657, 0x01, "[V40] developer_script");
	SafeWriteByte(0x88A3F7, 0x00, "[V40] lui_checksum_enabled");
	SafeWriteByte(0x88A5AF, 0x01, "[V40] xblive_rankedmatch");
	SafeWriteByte(0x5080D1, 0x2F, "[V40] silentfail");
	SafeWriteByte(0x5664CA, 0x00, "[V40] tu11_partymigrate");
	SafeWriteByte(0x544240, 0xC3, "[V40] WinMain patch");
	SafeWriteByte(0x6B6607, 0x01, "[V40] gpad_enabled");
	SafeWriteByte(0xC1D910, 0x33, "[V40] _tu14");
	SafeWriteByte(0xBE0F26, 0x33, "[V40] online_tu14_mp");
	SafeWriteByte(0x4BE550, 0xC3, "[V40] Steam menu patchF");
	SafeWriteByte(0x88A63F, 0x01, "[V40] developer");

	SafeStartConsoleThread((LPVOID)0x54D4F0);

	MainPatchFunctions::HKS_MaterialHook.Initialize(0x4863E0, MainPatchFunctions::HKS_RegisterMaterial);
	MainPatchFunctions::HKS_MaterialHook.InstallHook();

	MainPatchFunctions::HKS_StringHook.Initialize(0x895DB0, MainPatchFunctions::HKS_ReturnString);
	MainPatchFunctions::HKS_StringHook.InstallHook();

	MainPatchFunctions::HKS_RawfileHook.Initialize(0x494BA0, MainPatchFunctions::HKS_LoadRawfile);
	MainPatchFunctions::HKS_RawfileHook.InstallHook();

	MainPatchFunctions::ScriptParseHook.Initialize(0x5E7060, MainPatchFunctions::Load_ScriptParseTreeAsset);
	MainPatchFunctions::ScriptParseHook.InstallHook();

	MainPatchFunctions::LoadCustomWeaponAsset.Initialize(0x94E354, MainPatchFunctions::Load_WeaponAsset);
	MainPatchFunctions::LoadCustomWeaponAsset.InstallHook();

	MPLog(MP_COLOR_OK, "V40", "patch complete");
}

// ==========================================================
// V43 Multiplayer
// ==========================================================

void T6MP::PatchT6MP_V43()
{
	MPLog(MP_COLOR_INFO, "V43", "patch start");

	static cmd_function_s testHudElems_cmd;
	static cmd_function_s autoChangeClass_cmd;
	static cmd_function_s Notify_cmd;

	SafeAddCommand("testHudElems", testHudElems_f2, &testHudElems_cmd);
	SafeAddCommand("autoChangeClass", Bots::autoChangeClass_f, &autoChangeClass_cmd);
	SafeAddCommand("Notify", PopupNotify::Notify_f, &Notify_cmd);

	SafeWriteByte(0x517AFB, 0xEB, "[MP] fix jmp");
	SafeWriteByte(0x5DDC20, 0xC3, "[MP] unknown patch");
	SafeWriteByte(0x88BEA7, 0x00, "[MP] lui_checksum_enabled");
	SafeWriteByte(0x88C05F, 0x01, "[MP] xblive_rankedmatch");
	SafeWriteByte(0x62FA61, 0x2F, "[MP] silentfail");
	SafeWriteByte(0x4F0C0A, 0x00, "[MP] tu11_partymigrate");
	SafeWriteByte(0x6BAB80, 0xC3, "[MP] WinMain patch");
	SafeWriteByte(0x4E4DF7, 0x01, "[MP] gpad_enabled");
	SafeWriteByte(0x5E0660, 0xC3, "[MP] Steam menu patch");
	SafeWriteByte(0x88C107, 0x01, "[MP] developer_script");
	SafeWriteByte(0x88C0EF, 0x01, "[MP] developer");
	SafeWriteByte(0xBFF698, 0x33, "[MP] _tu17");
	SafeWriteByte(0xBF04AE, 0x33, "[MP] online_tu17_mp");

	SafeStartConsoleThread((LPVOID)0x473AD0);

	MainPatchFunctions::HKS_MaterialHook.Initialize(0x5152D0, MainPatchFunctions::HKS_RegisterMaterial);
	MainPatchFunctions::HKS_MaterialHook.InstallHook();

	MainPatchFunctions::HKS_StringHook.Initialize(0x897970, MainPatchFunctions::HKS_ReturnString);
	MainPatchFunctions::HKS_StringHook.InstallHook();

	MainPatchFunctions::HKS_RawfileHook.Initialize(0x69D680, MainPatchFunctions::HKS_LoadRawfile);
	MainPatchFunctions::HKS_RawfileHook.InstallHook();

	MainPatchFunctions::ScriptParseHook.Initialize(0x539630, MainPatchFunctions::Load_ScriptParseTreeAsset);
	MainPatchFunctions::ScriptParseHook.InstallHook();

	MainPatchFunctions::LoadCustomWeaponAsset.Initialize(0x950294, MainPatchFunctions::Load_WeaponAsset);
	MainPatchFunctions::LoadCustomWeaponAsset.InstallHook();

	MPLog(MP_COLOR_OK, "V43", "patch complete");
}

// ==========================================================
// V44 Command Thread
// ==========================================================

static DWORD WINAPI V44CommandThread(LPVOID)
{
	MPLog(MP_COLOR_INFO, "CMD", "command input ready");
	MPLog(MP_COLOR_INFO, "CMD", "commands: help, openConsole, forceOffline, initOverlay, quit");

	char input[256] = { 0 };

	while (true)
	{
		printf("> ");
		fflush(stdout);

		if (!fgets(input, sizeof(input), stdin))
		{
			Sleep(100);
			continue;
		}

		input[strcspn(input, "\r\n")] = 0;

		if (_stricmp(input, "help") == 0)
		{
			printf("Commands:\n");
			printf("  help\n");
			printf("  openConsole\n");
			printf("  forceOffline\n");
			printf("  initOverlay\n");
			printf("  quit\n");
		}
		else if (_stricmp(input, "openConsole") == 0)
		{
			openConsole_f();
		}
		else if (_stricmp(input, "forceOffline") == 0)
		{
			forceOffline_f();
		}
		else if (_stricmp(input, "initOverlay") == 0)
		{
			MPLog(MP_COLOR_INFO, "OVERLAY", "retrying InGameConsole::Initialize");

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
			MPLog(MP_COLOR_WARN, "CMD", "closing command thread");
			break;
		}
		else if (input[0])
		{
			MPLog(MP_COLOR_WARN, "CMD", "unknown command: %s", input);
		}

		memset(input, 0, sizeof(input));
		Sleep(10);
	}

	return 0;
}

// ==========================================================
// V44 Overlay Init
// ==========================================================

static DWORD WINAPI DelayedOverlayInitThread(LPVOID)
{
	MPLog(MP_COLOR_WARN, "OVERLAY", "waiting before overlay init...");

	Sleep(15000);

	MPLog(MP_COLOR_INFO, "OVERLAY", "auto initializing overlay...");

	__try
	{
		InGameConsole::Initialize();
		MPLog(MP_COLOR_OK, "OVERLAY", "initialized");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MPLog(MP_COLOR_ERROR, "OVERLAY", "init crashed");
	}

	return 0;
}

// ==========================================================
// V44 Multiplayer Safe Boot
// ==========================================================

void T6MP::PatchT6MP_V44()
{
	AllocConsole();

	FILE* dummy = NULL;
	freopen_s(&dummy, "CONOUT$", "w", stdout);
	freopen_s(&dummy, "CONOUT$", "w", stderr);
	freopen_s(&dummy, "CONIN$", "r", stdin);

	SetConsoleTitleA("MP V44 Safe Console");

	MPLog(MP_COLOR_INFO, "MP", "V44 debug console initialized");
	MPLog(MP_COLOR_WARN, "MP", "V44 SAFE BOOT ONLY");

	SteamCommon::LoadOverlay();
	DumpHandler::Initialize();
	DebugConsole::Initialize();

	HANDLE frontendThread = CreateThread(
		NULL,
		0,
		MP44FrontendWaitThread,
		NULL,
		0,
		NULL);

	if (frontendThread)
		CloseHandle(frontendThread);

	HANDLE commandThread = CreateThread(
		NULL,
		0,
		V44CommandThread,
		NULL,
		0,
		NULL);

	if (commandThread)
		CloseHandle(commandThread);

	// SERVER STUFF broken on V44 right now
	//ServerList::Initialize();
	//Server::Initialize();

	// DW hooks disabled while testing safe boot
	//SafeWriteDWORD(Addresses::dw_select, (DWORD)dw_Entry::dw_select, "[V44] dw_select");
	//SafeWriteDWORD(Addresses::dw_recv, (DWORD)dw_Entry::dw_recv, "[V44] dw_recv");
	//SafeWriteDWORD(Addresses::dw_send, (DWORD)dw_Entry::dw_send, "[V44] dw_send");
	//SafeWriteDWORD(Addresses::dw_connect, (DWORD)dw_Entry::dw_connect, "[V44] dw_connect");
	//SafeWriteDWORD(Addresses::dw_recvfrom, (DWORD)dw_Entry::dw_recvfrom, "[V44] dw_recvfrom");
	//SafeWriteDWORD(Addresses::dw_sendto, (DWORD)dw_Entry::dw_sendto, "[V44] dw_sendto");
	//SafeWriteDWORD(Addresses::custom_gethostbyname, (DWORD)dw_Entry::custom_gethostbyname, "[V44] custom_gethostbyname");

	// Game command registration crashes on V44 right now
	//static cmd_function_s testHudElems_cmd; SafeAddCommand("testHudElems", testHudElems_f2, &testHudElems_cmd);
	//static cmd_function_s Notify_cmd; SafeAddCommand("Notify", PopupNotify::Notify_f, &Notify_cmd);
	//static cmd_function_s autoChangeClass_cmd; SafeAddCommand("autoChangeClass", Bots::autoChangeClass_f, &autoChangeClass_cmd);
	//static cmd_function_s spawnBot_cmd; SafeAddCommand("spawnBot", Bots::spawnBot_f, &spawnBot_cmd);
	//static cmd_function_s forceOffline_cmd; SafeAddCommand("forceOffline", forceOffline_f, &forceOffline_cmd);
	//static cmd_function_s openConsole_cmd; SafeAddCommand("openConsole", openConsole_f, &openConsole_cmd);

	// Session mode commands caused script stack overflow / Lua context crashes
	//static cmd_function_s forceOnlineGame_cmd; SafeAddCommand("forceOnlineGame", forceOnlineGame_f, &forceOnlineGame_cmd);
	//static cmd_function_s forceSystemLink_cmd; SafeAddCommand("forceSystemLink", forceSystemLink_f, &forceSystemLink_cmd);
	//static cmd_function_s forceOfflineMode_cmd; SafeAddCommand("forceOfflineMode", forceOfflineMode_f, &forceOfflineMode_cmd);
	//static cmd_function_s forcePrivateMode_cmd; SafeAddCommand("forcePrivateMode", forcePrivateMode_f, &forcePrivateMode_cmd);

	HANDLE overlayThread = CreateThread(
		NULL,
		0,
		DelayedOverlayInitThread,
		NULL,
		0,
		NULL);

	if (overlayThread)
		CloseHandle(overlayThread);

	MPLog(MP_COLOR_OK, "MP", "V44 safe boot complete");
	MPLog(MP_COLOR_INFO, "MP", "Type help in this console");
}