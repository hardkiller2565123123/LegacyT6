#include "STDInc.h"

#define ZM42_DVAR_DEVELOPER_SCRIPT      0x025389F0
#define ZM42_DVAR_LUI_CHECKSUM_ENABLED  0x025B8FB8
#define ZM42_DVAR_XBLIVE_RANKEDMATCH    0x025B8DB0
#define ZM42_DVAR_DEVELOPER_SCRIPT      0x025389D0
#define ZM42_DVAR_LUI_CHECKSUM_ENABLED  0x025B9BF8
#define ZM42_DVAR_XBLIVE_RANKEDMATCH    0x025B8BD8
#define ZM42_DVAR_ONLINEGAME            0x025389F4
#define ZM42_DVAR_XBLIVE_PRIVATEMATCH   0x02538A44

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
			printf("%s VirtualProtect failed at 0x%08X err=%lu\n", name ? name : "ZMSafeWriteBytes", address, GetLastError());
			return false;
		}

		memcpy((void*)address, data, size);
		FlushInstructionCache(GetCurrentProcess(), (LPCVOID)address, size);

		DWORD temp = 0;
		VirtualProtect((LPVOID)address, size, oldProtect, &temp);

		printf("%s write OK -> 0x%08X size=%u\n", name ? name : "ZMSafeWriteBytes", address, (unsigned int)size);
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("%s write crashed -> 0x%08X size=%u\n", name ? name : "ZMSafeWriteBytes", address, (unsigned int)size);
		return false;
	}
}

static bool ZMSafeWriteByte(DWORD address, BYTE value, const char* name)
{
	return ZMSafeWriteBytes(address, &value, 1, name);
}

static bool ZMSafeReadDWORD(DWORD address, DWORD* out)
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

static bool ZMSafeSetDvarBoolByPointer(DWORD pointerAddress, bool enabled, const char* name)
{
	DWORD dvar = 0;

	if (!ZMSafeReadDWORD(pointerAddress, &dvar) || !dvar)
	{
		printf("[ZM V42] %s dvar pointer is NULL\n", name);
		return false;
	}

	BYTE value = enabled ? 1 : 0;

	ZMSafeWriteByte(dvar + 0x18, value, name);
	ZMSafeWriteByte(dvar + 0x1C, value, name);

	printf("[ZM V42] %s set -> ptr 0x%08X dvar 0x%08X value %d\n", name, pointerAddress, dvar, value);
	return true;
}

static void ForceZombieSystemLinkOnce()
{
	printf("[ZM V42] applying frontend dvar flags\n");

	ZMSafeSetDvarBoolByPointer(ZM42_DVAR_DEVELOPER_SCRIPT, true, "[ZM V42] developer_script");
	ZMSafeSetDvarBoolByPointer(ZM42_DVAR_LUI_CHECKSUM_ENABLED, false, "[ZM V42] lui_checksum_enabled");
	ZMSafeSetDvarBoolByPointer(ZM42_DVAR_XBLIVE_RANKEDMATCH, false, "[ZM V42] xblive_rankedmatch");
	ZMSafeSetDvarBoolByPointer(ZM42_DVAR_ONLINEGAME, false, "[ZM V42] onlinegame");
	ZMSafeSetDvarBoolByPointer(ZM42_DVAR_XBLIVE_PRIVATEMATCH, true, "[ZM V42] xblive_privatematch");

	printf("[ZM V42] frontend dvar pass finished\n");
}

static DWORD WINAPI ZMFrontendWaitThread(LPVOID)
{
	printf("[ZM V42] waiting for frontend dvars...\n");

	for (int i = 0; i < 600; i++)
	{
		DWORD dev = 0;
		DWORD lui = 0;
		DWORD ranked = 0;

		ZMSafeReadDWORD(ZM42_DVAR_DEVELOPER_SCRIPT, &dev);
		ZMSafeReadDWORD(ZM42_DVAR_LUI_CHECKSUM_ENABLED, &lui);
		ZMSafeReadDWORD(ZM42_DVAR_XBLIVE_RANKEDMATCH, &ranked);

		if (dev || lui || ranked)
		{
			printf("[ZM V42] frontend dvars found dev=0x%08X lui=0x%08X ranked=0x%08X\n", dev, lui, ranked);
			break;
		}

		Sleep(100);
	}

	for (int i = 0; i < 60; i++)
	{
		ForceZombieSystemLinkOnce();
		Sleep(500);
	}

	printf("[ZM V42] frontend dvar forcing finished\n");
	return 0;
}

static void ZMSafeAddCommand(const char* name, void(__cdecl* function)(), cmd_function_s* command)
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
		spawnTestElemZombie(i);
}

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
	ZMSafeWriteByte(0x88B157, 0x01, "[ZM V41] systemlink visible");

	ZMSafeStartConsoleThread((LPVOID)0x4A67F0);

	printf("[T6ZM] V41 safe patch complete\n");
}

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

	HANDLE frontendThread = CreateThread(NULL, 0, ZMFrontendWaitThread, NULL, 0, NULL);

	if (frontendThread)
		CloseHandle(frontendThread);

	HANDLE commandThread = CreateThread(NULL, 0, ZMV42CommandThread, NULL, 0, NULL);

	if (commandThread)
		CloseHandle(commandThread);

	printf("[T6ZM] V42 safe boot complete\n");
}