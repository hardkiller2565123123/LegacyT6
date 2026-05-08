#include "STDInc.h"

static void DebugLine(const char* text)
{
	if (!text) return;
	OutputDebugStringA(text);
	OutputDebugStringA("\n");
}

static bool SafeWriteByte(DWORD address, BYTE value, const char* name)
{
	if (!address)
	{
		DebugLine(name);
		DebugLine("address is NULL");
		return false;
	}

	__try
	{
		DWORD oldProtect = 0;

		if (!VirtualProtect((void*)address, 1, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			DebugLine(name);
			DebugLine("VirtualProtect failed");
			return false;
		}

		*(BYTE*)address = value;

		DWORD temp = 0;
		VirtualProtect((void*)address, 1, oldProtect, &temp);

		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DebugLine(name);
		DebugLine("write crashed");
		return false;
	}
}

static void SafeAddCommand(const char* name, void(__cdecl* function)(), cmd_function_s* command)
{
	if (!Addresses::Cmd_AddCommandInternal || !name || !function || !command)
		return;

	__try
	{
		Addresses::Cmd_AddCommandInternal(name, function, command);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DebugLine("[T6MP] Cmd_AddCommandInternal crashed");
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
		DebugLine("[T6MP] Console thread crashed");
	}
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

static bool SafeWriteDWORD(DWORD address, DWORD value, const char* name)
{
	if (!address)
	{
		DebugLine(name);
		DebugLine("address is NULL");
		return false;
	}

	__try
	{
		DWORD oldProtect = 0;

		if (!VirtualProtect((void*)address, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			DebugLine(name);
			DebugLine("VirtualProtect failed");
			return false;
		}

		*(DWORD*)address = value;

		DWORD temp = 0;
		VirtualProtect((void*)address, sizeof(DWORD), oldProtect, &temp);

		DebugLine(name);
		DebugLine("write OK");

		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DebugLine(name);
		DebugLine("write crashed");
		return false;
	}
}

void T6MP::PatchT6MP_V44()
{
	DebugLine("[T6MP] V44 SAFE BOOT ONLY");

	static cmd_function_s testHudElems_cmd;
	static cmd_function_s Notify_cmd;
	static cmd_function_s autoChangeClass_cmd;
	static cmd_function_s spawnBot_cmd;

	SafeAddCommand("testHudElems", testHudElems_f2, &testHudElems_cmd);
	SafeAddCommand("Notify", PopupNotify::Notify_f, &Notify_cmd);
	SafeAddCommand("autoChangeClass", Bots::autoChangeClass_f, &autoChangeClass_cmd);
	SafeAddCommand("spawnBot", Bots::spawnBot_f, &spawnBot_cmd);

	SafeStartConsoleThread((LPVOID)0x473AD0);

	SafeWriteDWORD(Addresses::dw_select, (DWORD)dw_Entry::dw_select, "[V44] dw_select");
	SafeWriteDWORD(Addresses::dw_recv, (DWORD)dw_Entry::dw_recv, "[V44] dw_recv");
	SafeWriteDWORD(Addresses::dw_send, (DWORD)dw_Entry::dw_send, "[V44] dw_send");
	SafeWriteDWORD(Addresses::dw_connect, (DWORD)dw_Entry::dw_connect, "[V44] dw_connect");
	SafeWriteDWORD(Addresses::dw_recvfrom, (DWORD)dw_Entry::dw_recvfrom, "[V44] dw_recvfrom");
	SafeWriteDWORD(Addresses::dw_sendto, (DWORD)dw_Entry::dw_sendto, "[V44] dw_sendto");

	SafeWriteDWORD(
		Addresses::custom_gethostbyname,
		(DWORD)dw_Entry::custom_gethostbyname,
		"[V44] custom_gethostbyname");

	SteamCommon::LoadOverlay();
	DumpHandler::Initialize();

	// Still unsafe on V44
	//CegHandler::Initialize();

	ServerList::Initialize();
	Server::Initialize();
}