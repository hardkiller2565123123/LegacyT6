#include "STDPatches.h"

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
