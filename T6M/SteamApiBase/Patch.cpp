#include "STDInc.h"
#include "CEG.h"

void Patch()
{
	switch (*(DWORD*)0x41136F)
	{
		// ============================================
		// Version 39 - Zombies
		// ============================================
	case 0x75C08500:
	{
		Global::Game::Version = GAME_VERSION_39;
		Global::Game::Type = GAME_TYPE_ZM;
		break;
	}

	// ============================================
	// Version 40 - Multiplayer
	// ============================================
	case 0x111F9C35:
	{
		Global::Game::Version = GAME_VERSION_40;
		Global::Game::Type = GAME_TYPE_MP;
		break;
	}

	// ============================================
	// Version 41 - Zombies
	// ============================================
	case 0x244C8BCC:
	{
		Global::Game::Version = GAME_VERSION_41;
		Global::Game::Type = GAME_TYPE_ZM;
		break;
	}

	// ============================================
	// Version 43 - Multiplayer
	// ============================================
	case 0x30C4835E:
	{
		Global::Game::Version = GAME_VERSION_43;
		Global::Game::Type = GAME_TYPE_MP;
		break;
	}

	// ============================================
	// Version 44 - Multiplayer
	// ============================================
	case 0x1AE85508:
	{
		Global::Game::Version = GAME_VERSION_44;
		Global::Game::Type = GAME_TYPE_MP;
		break;
	}

	// ============================================
	// Version 44 - Zombies
	// ============================================
	case 0x5E8B3046:
	{
		Global::Game::Version = GAME_VERSION_44;
		Global::Game::Type = GAME_TYPE_ZM;
		break;
	}

	// ============================================
	// Dedicated Debug
	// ============================================
	case 0x0F0C7D8B:
	{
		Global::Game::Version = GAME_VERSION_DEDI_DEBUG;
		Global::Game::Type = GAME_TYPE_DEDI;
		break;
	}

	// ============================================
	// Unknown Version
	// ============================================
	default:
	{
		char buffer[128];

		sprintf_s(
			buffer,
			"Unsupported game version.\n\nSignature: 0x%08X",
			*(DWORD*)0x41136F);

		MessageBoxA(
			0,
			buffer,
			"PlusOps Version Check",
			MB_ICONERROR | MB_OK);

		Global::Game::Version = GAME_VERSION_ERROR;
		Global::Game::Type = GAME_TYPE_ERROR;
		break;
	}
	}

	// ============================================
	// Base Initialization
	// ============================================

	Addresses::Assign();

	/*
		V44 SAFE STARTUP

		Only safe systems enabled right now.

		Disabled:
		- CEG
		- ServerList
		- DW Server
		- Old V43/V41 byte patches
		- Old hooks
	*/



	// ============================================
	// Zombies
	// ============================================

	if (Global::Game::Type == GAME_TYPE_ZM)
	{
		Global::Variables::Steam_AppID = 212910;

		if (Global::Game::Version == GAME_VERSION_39)
		{
			T6ZM::PatchT6ZM_V39();
		}
		else if (Global::Game::Version == GAME_VERSION_41)
		{
			T6ZM::PatchT6ZM_V41();
		}
		else if (Global::Game::Version == GAME_VERSION_44)
		{
			OutputDebugStringA("[Patch] V44 ZM SAFE BOOT\n");

			/*
				Temporary shared V44 startup.

				Do NOT use old V41 zombie offsets yet.
			*/

			T6MP::PatchT6MP_V44();
		}
	}

	// ============================================
	// Multiplayer
	// ============================================

	else if (Global::Game::Type == GAME_TYPE_MP)
	{
		Global::Variables::Steam_AppID = 202990;

		if (Global::Game::Version == GAME_VERSION_40)
		{
			T6MP::PatchT6MP_V40();
		}
		else if (Global::Game::Version == GAME_VERSION_43)
		{
			T6MP::PatchT6MP_V43();
		}
		else if (Global::Game::Version == GAME_VERSION_44)
		{
			OutputDebugStringA("[Patch] V44 MP SAFE BOOT\n");

			/*
				V44 currently runs in safe mode only.

				No:
				- byte patches
				- hooks
				- command registration
				- CEG
				- server hooks
			*/

			T6MP::PatchT6MP_V44();
		}
	}

	// ============================================
	// Invalid
	// ============================================

	else
	{
		Auth_Error("Game version not supported. Aborting startup");
		ExitProcess(0x8000D3AD);
	}
}