#include "STDInc.h"

/*
	V44 BAD FUNCTIONS / PATCHES - DO NOT ENABLE WITHOUT PROPER OFFSETS
*/

// Byte patches
//SafeWriteByte(0x88C0EF, 0x01, "[V44] developer");
//SafeWriteByte(0x88C107, 0x01, "[V44] developer_script");
//SafeWriteByte(0x88BEA7, 0x00, "[V44] lui_checksum_enabled");
//SafeWriteByte(0x88C05F, 0x01, "[V44] xblive_rankedmatch");
//SafeWriteByte(0x4E4DF7, 0x01, "[V44] gpad_enabled");

// Hooks
//MainPatchFunctions::HKS_MaterialHook.Initialize(0x5152D0, MainPatchFunctions::HKS_RegisterMaterial);
//MainPatchFunctions::HKS_MaterialHook.InstallHook();

//MainPatchFunctions::HKS_StringHook.Initialize(0x897970, MainPatchFunctions::HKS_ReturnString);
//MainPatchFunctions::HKS_StringHook.InstallHook();

//MainPatchFunctions::HKS_RawfileHook.Initialize(0x69D680, MainPatchFunctions::HKS_LoadRawfile);
//MainPatchFunctions::HKS_RawfileHook.InstallHook();

//MainPatchFunctions::ScriptParseHook.Initialize(0x539630, MainPatchFunctions::Load_ScriptParseTreeAsset);
//MainPatchFunctions::ScriptParseHook.InstallHook();

//MainPatchFunctions::LoadCustomWeaponAsset.Initialize(0x950294, MainPatchFunctions::Load_WeaponAsset);
//MainPatchFunctions::LoadCustomWeaponAsset.InstallHook();

//Addresses::Cbuf_AddText(0, "set ui_netSource 1\n");

// Systems
//CegHandler::Initialize();
//ServerList::Initialize();
//Server::Initialize();