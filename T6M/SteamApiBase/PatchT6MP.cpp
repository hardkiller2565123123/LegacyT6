#include "STDInc.h"
#include "../../../PlusOpsSource/PlusOpsSource/SteamApiBase/CEG.h"
#include "DebugConsole.h"

static void DebugLine(const char* text)
{
	if (!text)
		return;

	OutputDebugStringA(text);
	OutputDebugStringA("\n");
}

static bool SafeWriteByte(DWORD address, BYTE value, const char* name)
{
	if (!address)
	{
		printf("%s address is NULL\n", name ? name : "SafeWriteByte");
		return false;
	}

	__try
	{
		DWORD oldProtect = 0;

		if (!VirtualProtect((void*)address, 1, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			printf("%s VirtualProtect failed\n", name ? name : "SafeWriteByte");
			return false;
		}

		*(BYTE*)address = value;

		DWORD temp = 0;
		VirtualProtect((void*)address, 1, oldProtect, &temp);

		printf("%s write OK -> 0x%08X\n", name ? name : "SafeWriteByte", address);
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("%s write crashed -> 0x%08X\n", name ? name : "SafeWriteByte", address);
		return false;
	}
}

static bool SafeWriteDWORD(DWORD address, DWORD value, const char* name)
{
	if (!address)
	{
		printf("%s address is NULL\n", name ? name : "SafeWriteDWORD");
		return false;
	}

	__try
	{
		DWORD oldProtect = 0;

		if (!VirtualProtect((void*)address, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			printf("%s VirtualProtect failed\n", name ? name : "SafeWriteDWORD");
			return false;
		}

		*(DWORD*)address = value;

		DWORD temp = 0;
		VirtualProtect((void*)address, sizeof(DWORD), oldProtect, &temp);

		printf("%s write OK -> 0x%08X\n", name ? name : "SafeWriteDWORD", address);
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("%s write crashed -> 0x%08X\n", name ? name : "SafeWriteDWORD", address);
		return false;
	}
}

static void SafeAddCommand(
	const char* name,
	void(__cdecl* function)(),
	cmd_function_s* command)
{
	if (!Addresses::Cmd_AddCommandInternal || !name || !function || !command)
	{
		printf("[T6MP] SafeAddCommand skipped: %s\n", name ? name : "NULL");
		return;
	}

	__try
	{
		Addresses::Cmd_AddCommandInternal(name, function, command);
		printf("[T6MP] command registered: %s\n", name);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[T6MP] Cmd_AddCommandInternal crashed: %s\n", name);
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
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[T6MP] Console thread crashed\n");
	}
}

static void SafeCbufAddText(const char* text)
{
	if (!Addresses::Cbuf_AddText || !text)
	{
		printf("[V44] Cbuf_AddText missing\n");
		return;
	}

	__try
	{
		printf("[V44] Cbuf_AddText: %s\n", text);
		Addresses::Cbuf_AddText(0, text);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[V44] Cbuf_AddText crashed: %s\n", text);
	}
}

typedef void(__cdecl* SessionModeFunc_t)();

static void SafeCallSessionMode(DWORD address, const char* name)
{
	if (!address)
	{
		printf("[V44] %s address is NULL\n", name ? name : "SessionMode");
		return;
	}

	__try
	{
		printf("[V44] calling %s at 0x%08X\n", name ? name : "SessionMode", address);

		SessionModeFunc_t fn = (SessionModeFunc_t)address;
		fn();

		printf("[V44] %s finished\n", name ? name : "SessionMode");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[V44] %s crashed\n", name ? name : "SessionMode");
	}
}

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
	printf("[V44] forcing offline / LAN menu state\n");

	SafeCbufAddText("set onlinegame 0\n");
	SafeCbufAddText("set systemlink 1\n");
	SafeCbufAddText("set ui_netSource 1\n");
	SafeCbufAddText("set xblive_privatematch 1\n");
	SafeCbufAddText("set xblive_loggedin 0\n");
	SafeCbufAddText("disconnect\n");
	SafeCbufAddText("openmenu menu_systemlink_lobby\n");
}

void openConsole_f()
{
	printf("[V44] forcing console dvars + toggleconsole\n");

	SafeCbufAddText("set developer 1\n");
	SafeCbufAddText("set monkeytoy 0\n");
	SafeCbufAddText("set con_minicon 1\n");
	SafeCbufAddText("toggleconsole\n");
}

void spawnTestElem(int client)
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

void T6MP::PatchT6MP_V40()
{
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
	SafeWriteByte(0x4BE550, 0xC3, "[V40] Steam menu patch");
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
}

void T6MP::PatchT6MP_V43()
{
	static cmd_function_s testHudElems_cmd;
	static cmd_function_s autoChangeClass_cmd;
	static cmd_function_s Notify_cmd;

	SafeAddCommand("testHudElems", testHudElems_f2, &testHudElems_cmd);
	SafeAddCommand("autoChangeClass", Bots::autoChangeClass_f, &autoChangeClass_cmd);
	SafeAddCommand("Notify", PopupNotify::Notify_f, &Notify_cmd);

	SafeWriteByte(0x517AFB, 0xEB, "[V43] fix jmp");
	SafeWriteByte(0x5DDC20, 0xC3, "[V43] unknown patch");
	SafeWriteByte(0x88BEA7, 0x00, "[V43] lui_checksum_enabled");
	SafeWriteByte(0x88C05F, 0x01, "[V43] xblive_rankedmatch");
	SafeWriteByte(0x62FA61, 0x2F, "[V43] silentfail");
	SafeWriteByte(0x4F0C0A, 0x00, "[V43] tu11_partymigrate");
	SafeWriteByte(0x6BAB80, 0xC3, "[V43] WinMain patch");
	SafeWriteByte(0x4E4DF7, 0x01, "[V43] gpad_enabled");
	SafeWriteByte(0x5E0660, 0xC3, "[V43] Steam menu patch");
	SafeWriteByte(0x88C107, 0x01, "[V43] developer_script");
	SafeWriteByte(0x88C0EF, 0x01, "[V43] developer");
	SafeWriteByte(0xBFF698, 0x33, "[V43] _tu17");
	SafeWriteByte(0xBF04AE, 0x33, "[V43] online_tu17_mp");

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
}


static DWORD WINAPI V44CommandThread(LPVOID)
{
	printf("[V44] command input ready\n");
	printf("[V44] commands: help, openConsole, forceOffline, quit\n");

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
		else if (_stricmp(input, "quit") == 0)
		{
			printf("[V44] closing command thread\n");
			break;
		}
		else if (input[0])
		{
			printf("[V44] unknown command: %s\n", input);
		}

		memset(input, 0, sizeof(input));
		Sleep(10);
	}

	return 0;
}





void T6MP::PatchT6MP_V44()
{
	// Initialize console first
	AllocConsole();

	FILE* dummy = NULL;
	freopen_s(&dummy, "CONOUT$", "w", stdout);
	freopen_s(&dummy, "CONOUT$", "w", stderr);
	freopen_s(&dummy, "CONIN$", "r", stdin);

	SetConsoleTitleA("T6M V44 Safe Console");

	printf("[T6MP] V44 debug console initialized\n");
	printf("[T6MP] V44 SAFE BOOT ONLY\n");

	// Safe systems
	SteamCommon::LoadOverlay();
	DumpHandler::Initialize();

	// Optional clean logger
	//DebugConsole::Initialize();

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

	// Session mode commands caused script stack overflow
	//static cmd_function_s forceOnlineGame_cmd; SafeAddCommand("forceOnlineGame", forceOnlineGame_f, &forceOnlineGame_cmd);
	//static cmd_function_s forceSystemLink_cmd; SafeAddCommand("forceSystemLink", forceSystemLink_f, &forceSystemLink_cmd);
	//static cmd_function_s forceOfflineMode_cmd; SafeAddCommand("forceOfflineMode", forceOfflineMode_f, &forceOfflineMode_cmd);
	//static cmd_function_s forcePrivateMode_cmd; SafeAddCommand("forcePrivateMode", forcePrivateMode_f, &forcePrivateMode_cmd);

	printf("[V44] testing InGameConsole::Initialize\n");

	__try
	{
		InGameConsole::Initialize();
		printf("[V44] InGameConsole initialized\n");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[V44] InGameConsole crashed\n");
	}

	HANDLE commandThread = CreateThread(
		NULL,
		0,
		V44CommandThread,
		NULL,
		0,
		NULL);

	if (commandThread)
		CloseHandle(commandThread);

	printf("[T6MP] V44 safe boot complete\n");
	printf("[T6MP] Type help in this console\n");
}