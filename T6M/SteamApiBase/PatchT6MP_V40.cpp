#include "STDPatches.h"

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
