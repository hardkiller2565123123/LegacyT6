#include "STDInc.h"

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

	if (Addresses::SV_Loaded())
	{
		for (int i = 0; i < 4; i++)
		{
			spawnTestElem(i);
		}
	}
}

static void SafeWriteByte(DWORD address, BYTE value, const char* name)
{
	if (!address)
	{
		OutputDebugStringA(name);
		OutputDebugStringA(" address is NULL\n");
		return;
	}

	__try
	{
		DWORD oldProtect = 0;

		if (VirtualProtect((void*)address, 1, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			*(BYTE*)address = value;

			DWORD temp = 0;
			VirtualProtect((void*)address, 1, oldProtect, &temp);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		OutputDebugStringA(name);
		OutputDebugStringA(" write crashed\n");
	}
}

static void SafeAddCommand(const char* name, void(__cdecl* function)(), cmd_function_s* command)
{
	if (!Addresses::Cmd_AddCommandInternal)
	{
		OutputDebugStringA("[T6MP] Cmd_AddCommandInternal is NULL\n");
		return;
	}

	if (!name || !function || !command)
		return;

	__try
	{
		Addresses::Cmd_AddCommandInternal(name, function, command);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		OutputDebugStringA("[T6MP] Cmd_AddCommandInternal crashed\n");
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

	SafeWriteByte(0x5BFA6B, 0xEB, "fix jmp");
	SafeWriteByte(0x5DDC20, 0xC3, "unknown patch");
	SafeWriteByte(0x88A657, 0x01, "developer_script");
	SafeWriteByte(0x88A3F7, 0x00, "lui_checksum_enabled");
	SafeWriteByte(0x88A5AF, 0x01, "xblive_rankedmatch");
	SafeWriteByte(0x5080D1, 0x2F, "silentfail");
	SafeWriteByte(0x5664CA, 0x00, "tu11_partymigrate");
	SafeWriteByte(0x544240, 0xC3, "WinMain patch");
	SafeWriteByte(0x6B6607, 0x01, "gpad_enabled");
	SafeWriteByte(0xC1D910, 0x33, "_tu14");
	SafeWriteByte(0xBE0F26, 0x33, "online_tu14_mp");
	SafeWriteByte(0x4BE550, 0xC3, "Steam menu patch");
	SafeWriteByte(0x88A63F, 0x01, "developer");

	CreateThread(0, 0, ConsoleWindow, (LPVOID)0x54D4F0, 0, 0);

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

	SafeWriteByte(0x517AFB, 0xEB, "fix jmp");
	SafeWriteByte(0x5DDC20, 0xC3, "unknown patch");
	SafeWriteByte(0x88BEA7, 0x00, "lui_checksum_enabled");
	SafeWriteByte(0x88C05F, 0x01, "xblive_rankedmatch");
	SafeWriteByte(0x62FA61, 0x2F, "silentfail");
	SafeWriteByte(0x4F0C0A, 0x00, "tu11_partymigrate");
	SafeWriteByte(0x6BAB80, 0xC3, "WinMain patch");
	SafeWriteByte(0x4E4DF7, 0x01, "gpad_enabled");
	SafeWriteByte(0x5E0660, 0xC3, "Steam menu patch");
	SafeWriteByte(0x88C107, 0x01, "developer_script");
	SafeWriteByte(0x88C0EF, 0x01, "developer");
	SafeWriteByte(0xBFF698, 0x33, "_tu17");
	SafeWriteByte(0xBF04AE, 0x33, "online_tu17_mp");

	CreateThread(0, 0, ConsoleWindow, (LPVOID)0x473AD0, 0, 0);

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

void T6MP::PatchT6MP_V44()
{
	OutputDebugStringA("[T6MP] V44 SAFE BOOT\n");

	CreateThread(0, 0, ConsoleWindow, (LPVOID)0x473AD0, 0, 0);
}