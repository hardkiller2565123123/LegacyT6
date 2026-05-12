#include "STDInc.h"

// ==========================================================
// ZM Safe Helpers
// ==========================================================

static void ZMPrint(const char* text)
{
	if (!text)
		return;

	printf("%s\n", text);
	OutputDebugStringA(text);
	OutputDebugStringA("\n");
}

static bool ZMSafeWriteBytes(DWORD address, const BYTE* data, SIZE_T size, const char* name)
{
	if (!address || !data || !size)
	{
		printf("%s invalid args\n", name ? name : "ZMSafeWriteBytes");
		return false;
	}

	__try
	{
		DWORD oldProtect = 0;

		if (!VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			printf(
				"%s VirtualProtect failed at 0x%08X err=%lu\n",
				name ? name : "ZMSafeWriteBytes",
				address,
				GetLastError());

			return false;
		}

		memcpy((void*)address, data, size);
		FlushInstructionCache(GetCurrentProcess(), (LPCVOID)address, size);

		DWORD temp = 0;
		VirtualProtect((LPVOID)address, size, oldProtect, &temp);

		printf(
			"%s write OK -> 0x%08X size=%u\n",
			name ? name : "ZMSafeWriteBytes",
			address,
			(unsigned int)size);

		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf(
			"%s write crashed -> 0x%08X size=%u\n",
			name ? name : "ZMSafeWriteBytes",
			address,
			(unsigned int)size);

		return false;
	}
}

static bool ZMSafeWriteByte(DWORD address, BYTE value, const char* name)
{
	return ZMSafeWriteBytes(address, &value, 1, name);
}

static bool ZMSafeWriteDWORD(DWORD address, DWORD value, const char* name)
{
	return ZMSafeWriteBytes(address, (const BYTE*)&value, sizeof(value), name);
}

static bool ZMSafePatchJumpShort(DWORD address, const char* name)
{
	BYTE patch = 0xEB;
	return ZMSafeWriteByte(address, patch, name);
}

static bool ZMSafeNop(DWORD address, SIZE_T size, const char* name)
{
	BYTE buffer[32] = { 0 };

	if (size > sizeof(buffer))
		size = sizeof(buffer);

	memset(buffer, 0x90, size);
	return ZMSafeWriteBytes(address, buffer, size, name);
}

static void ZMSafeAddCommand(
	const char* name,
	void(__cdecl* function)(),
	cmd_function_s* command)
{
	if (!Addresses::Cmd_AddCommandInternal || !name || !function || !command)
	{
		printf("[T6ZM] SafeAddCommand skipped: %s\n", name ? name : "NULL");
		return;
	}

	__try
	{
		Addresses::Cmd_AddCommandInternal(name, function, command);
		printf("[T6ZM] command registered: %s\n", name);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[T6ZM] Cmd_AddCommandInternal crashed: %s\n", name);
	}
}

static void ZMSafeStartConsoleThread(LPVOID startupAddress)
{
	if (!startupAddress)
		return;

	__try
	{
		HANDLE thread = CreateThread(0, 0, ConsoleWindow, startupAddress, 0, 0);

		if (thread)
			CloseHandle(thread);

		printf("[T6ZM] console thread started -> 0x%08X\n", startupAddress);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[T6ZM] Console thread crashed\n");
	}
}

// ==========================================================
// V42 / Zombies V44 Local/Systemlink Bootstrap
// ==========================================================

static void ForceZombieSystemLink()
{
	printf("[ZM V42] forcing systemlink/frontend flags\n");
	printf("[ZM V42] This Is For Testing Does Not Work\n");

	// NOTE:
	// These are currently test candidates. If any of these crash or break boot,
	// comment them one-by-one. Do not patch 0x004D5CF3 anymore.

	//ZMSafeWriteByte(0x0088B16F, 0x01, "[ZM V42] developer_script");
	//ZMSafeWriteByte(0x0088AF0F, 0x00, "[ZM V42] lui_checksum_enabled");
	//ZMSafeWriteByte(0x0088B0C7, 0x01, "[ZM V42] xblive_rankedmatch");

	// Old V41 systemlink visible candidate.
	// In your V42 IDA this address landed in code before, so keep it disabled.
	// ZMSafeWriteByte(0x0088B157, 0x01, "[ZM V42] systemlink visible");

	// Old V41 steam restart bypass candidate.
	// Keep disabled until confirmed in V42 IDA.
	// ZMSafeWriteByte(0x0052E240, 0xC3, "[ZM V42] no Steam restart");

	printf("[ZM V42] systemlink/frontend flag pass finished\n");
}

// ==========================================================
// Optional V42 Command Thread
// ==========================================================

static DWORD WINAPI ZMV42CommandThread(LPVOID)
{
	printf("[T6ZM] command input ready\n");
	printf("[T6ZM] commands: help, quit\n");

	char input[256] = { 0 };

	while (true)
	{
		printf("ZM> ");
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
			printf("  quit\n");
		}
		else if (_stricmp(input, "quit") == 0)
		{
			printf("[T6ZM] closing command thread\n");
			break;
		}
		else if (input[0])
		{
			printf("[T6ZM] unknown command: %s\n", input);
		}

		memset(input, 0, sizeof(input));
		Sleep(10);
	}

	return 0;
}

// ==========================================================
// HUD Test
// ==========================================================

void spawnTestElemZombie(int client)
{
	__try
	{
		if (!Addresses::HudElem_Alloc)
		{
			printf("[T6ZM] HudElem_Alloc missing\n");
			return;
		}

		HudElem* elem = HudElements::HudElem_Allocate();

		if (!elem)
			return;

		HudElements::HudElem_Init(elem, client);
		HudElements::HudElem_SetText(elem, "T6ZM HUD test loaded");
		HudElements::HudElem_SetAlign(elem, TOP_LEFT);
		HudElements::HudElem_SetColor(elem, 0, 255, 255, 255);
		HudElements::HudElem_SetGlow(elem, 0, 255, 255, 40);

		elem->fontScale = 2.0f;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("[T6ZM] spawnTestElemZombie crashed\n");
	}
}

void testHudElems_f()
{
	if (!Addresses::SV_Loaded)
	{
		printf("[T6ZM] SV_Loaded missing\n");
		return;
	}

	if (!Addresses::SV_Loaded())
	{
		printf("[T6ZM] server not loaded\n");
		return;
	}

	for (int i = 0; i < 4; i++)
	{
		spawnTestElemZombie(i);
	}
}

// ==========================================================
// V39 Zombies
// ==========================================================

void T6ZM::PatchT6ZM_V39()
{
	printf("[T6ZM] V39 safe patch start\n");

	ZMSafeWriteByte(0x5F6AFB, 0xEB, "[ZM V39] fix jmp");
	ZMSafeWriteByte(0x46DF20, 0xC3, "[ZM V39] return shield");
	ZMSafeWriteByte(0x87E29F, 0x01, "[ZM V39] developer_script");
	ZMSafeWriteByte(0x87E03F, 0x00, "[ZM V39] lui_checksum_enabled");
	ZMSafeWriteByte(0x87E1F7, 0x01, "[ZM V39] xblive_rankedmatch");
	ZMSafeWriteByte(0x552841, 0x2F, "[ZM V39] silentfail");
	ZMSafeWriteByte(0x5D2E20, 0xC3, "[ZM V39] shield");
	ZMSafeWriteByte(0x4187A7, 0x01, "[ZM V39] gpad_enabled");
	ZMSafeWriteByte(0x676330, 0xC3, "[ZM V39] no Steam restart");
	ZMSafeWriteByte(0x87E287, 0x01, "[ZM V39] systemlink visible");

	ZMSafeStartConsoleThread((LPVOID)0x5CBFB0);

	printf("[T6ZM] V39 safe patch complete\n");
}

// ==========================================================
// V41 Zombies
// ==========================================================

void T6ZM::PatchT6ZM_V41()
{
	AllocConsole();

	FILE* dummy = NULL;
	freopen_s(&dummy, "CONOUT$", "w", stdout);
	freopen_s(&dummy, "CONOUT$", "w", stderr);
	freopen_s(&dummy, "CONIN$", "r", stdin);

	SetConsoleTitleA("T6ZM V41 Safe Console");

	printf("[T6ZM] V41 safe patch start\n");

	static cmd_function_s autoChangeClass_cmd;
	static cmd_function_s testHudElems_cmd;
	static cmd_function_s Notify_cmd;
	static cmd_function_s spawnBot_cmd;

	ZMSafeAddCommand("autoChangeClass", Bots::autoChangeClass_f, &autoChangeClass_cmd);
	ZMSafeAddCommand("testHudElems", testHudElems_f, &testHudElems_cmd);
	ZMSafeAddCommand("Notify", PopupNotify::Notify_f, &Notify_cmd);
	ZMSafeAddCommand("spawnBot", Bots::spawnBot_f, &spawnBot_cmd);

	ZMSafeWriteByte(0xC0BF44, 0x33, "[ZM V41] _tu15");
	ZMSafeWriteByte(0xC27F96, 0x33, "[ZM V41] online_tu15_zm");

	ZMSafeWriteByte(0x46049B, 0xEB, "[ZM V41] fix jmp");
	ZMSafeWriteByte(0x477830, 0xC3, "[ZM V41] return shield");
	ZMSafeWriteByte(0x88B16F, 0x01, "[ZM V41] developer_script");
	ZMSafeWriteByte(0x88AF0F, 0x00, "[ZM V41] lui_checksum_enabled");
	ZMSafeWriteByte(0x88B0C7, 0x01, "[ZM V41] xblive_rankedmatch");
	ZMSafeWriteByte(0x6B75F1, 0x2F, "[ZM V41] silentfail");
	ZMSafeWriteByte(0x665410, 0xC3, "[ZM V41] shield");
	ZMSafeWriteByte(0x668887, 0x01, "[ZM V41] gpad_enabled");
	ZMSafeWriteByte(0x52E240, 0xC3, "[ZM V41] no Steam restart");

	// Safe only for real V41. Do not copy to V42.
	ZMSafeWriteByte(0x88B157, 0x01, "[ZM V41] systemlink visible");

	ZMSafeStartConsoleThread((LPVOID)0x4A67F0);

	// Keep disabled until safe boot is confirmed.
	//MainPatchFunctions::HKS_MaterialHook.Initialize(0x6D0080, MainPatchFunctions::HKS_RegisterMaterial);
	//MainPatchFunctions::HKS_MaterialHook.InstallHook();

	//MainPatchFunctions::HKS_StringHook.Initialize(0x8964F0, MainPatchFunctions::HKS_ReturnString);
	//MainPatchFunctions::HKS_StringHook.InstallHook();

	//MainPatchFunctions::HKS_RawfileHook.Initialize(0x4A7550, MainPatchFunctions::HKS_LoadRawfile);
	//MainPatchFunctions::HKS_RawfileHook.InstallHook();

	//MainPatchFunctions::ScriptParseHook.Initialize(0x4F8E50, MainPatchFunctions::Load_ScriptParseTreeAsset);
	//MainPatchFunctions::ScriptParseHook.InstallHook();

	printf("[T6ZM] V41 safe patch complete\n");
}

// ==========================================================
// V42 / Zombies V44
// ==========================================================

void T6ZM::PatchT6ZM_V42()
{
	AllocConsole();

	FILE* dummy = NULL;
	freopen_s(&dummy, "CONOUT$", "w", stdout);
	freopen_s(&dummy, "CONOUT$", "w", stderr);
	freopen_s(&dummy, "CONIN$", "r", stdin);

	SetConsoleTitleA("T6ZM V42 Safe Console");

	printf("[T6ZM] V42 debug console initialized\n");
	printf("[T6ZM] V42 SAFE BOOT ONLY\n");

	SteamCommon::LoadOverlay();
	DumpHandler::Initialize();

	ForceZombieSystemLink();

	HANDLE commandThread = CreateThread(
		NULL,
		0,
		ZMV42CommandThread,
		NULL,
		0,
		NULL);

	if (commandThread)
		CloseHandle(commandThread);

	printf("[T6ZM] V42 safe boot complete\n");
}