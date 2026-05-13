#include "STDInc.h"
#include "CEG.h"
#include "STDPatches.h"

static DWORD GetGameSignature()
{
	__try
	{
		return *(DWORD*)0x41136F;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return 0;
	}
}

static void GetCurrentExePath(char* buffer, size_t size)
{
	if (!buffer || size == 0)
		return;

	buffer[0] = 0;

	GetModuleFileNameA(NULL, buffer, (DWORD)size);
	_strlwr_s(buffer, size);
}

static bool CurrentExeContains(const char* text)
{
	if (!text)
		return false;

	char path[MAX_PATH] = { 0 };
	char needle[128] = { 0 };

	GetCurrentExePath(path, sizeof(path));

	strcpy_s(needle, sizeof(needle), text);
	_strlwr_s(needle, sizeof(needle));

	return strstr(path, needle) != NULL;
}

static bool IsZombiesExe()
{
	return CurrentExeContains("t6zm") || CurrentExeContains("zombie");
}

static bool IsMultiplayerExe()
{
	return CurrentExeContains("t6mp") || CurrentExeContains("multiplayer");
}

static void DetectVersionAndType()
{
	DWORD signature = GetGameSignature();

	switch (signature)
	{
	case 0x75C08500:
		Global::Game::Version = GAME_VERSION_39;
		Global::Game::Type = GAME_TYPE_ZM;
		break;

	case 0x111F9C35:
		Global::Game::Version = GAME_VERSION_40;
		Global::Game::Type = GAME_TYPE_MP;
		break;

	case 0x244C8BCC:
		Global::Game::Version = GAME_VERSION_41;
		Global::Game::Type = GAME_TYPE_ZM;
		break;

	case 0x5E8B3046:
		Global::Game::Version = GAME_VERSION_42;
		Global::Game::Type = GAME_TYPE_ZM;
		break;

	case 0x30C4835E:
		Global::Game::Version = GAME_VERSION_43;
		Global::Game::Type = GAME_TYPE_MP;
		break;

	case 0x1AE85508:
		Global::Game::Version = GAME_VERSION_44;
		Global::Game::Type = GAME_TYPE_MP;
		break;

	case 0x0F0C7D8B:
		Global::Game::Version = GAME_VERSION_DEDI_DEBUG;
		Global::Game::Type = GAME_TYPE_DEDI;
		break;

	default:
		Global::Game::Version = GAME_VERSION_ERROR;
		Global::Game::Type = GAME_TYPE_ERROR;
		break;
	}

	if (IsZombiesExe())
		Global::Game::Type = GAME_TYPE_ZM;
	else if (IsMultiplayerExe())
		Global::Game::Type = GAME_TYPE_MP;
}

static void ShowUnsupportedVersion()
{
	DWORD signature = GetGameSignature();

	char buffer[256] = { 0 };

	sprintf_s(
		buffer,
		sizeof(buffer),
		"Unsupported game version.\n\nSignature: 0x%08X",
		signature);

	MessageBoxA(
		0,
		buffer,
		"PlusOps Version Check",
		MB_ICONERROR | MB_OK);
}

void Patch()
{
	DetectVersionAndType();

	if (Global::Game::Version == GAME_VERSION_ERROR ||
		Global::Game::Type == GAME_TYPE_ERROR)
	{
		ShowUnsupportedVersion();
		Auth_Error("Game version not supported. Aborting startup");
		ExitProcess(0x8000D3AD);
		return;
	}

	Addresses::Assign();

	if (Global::Game::Type == GAME_TYPE_ZM)
	{
		Global::Variables::Steam_AppID = 212910;
		OutputDebugStringA("[Patch] Zombies exe detected\n");

		if (Global::Game::Version == GAME_VERSION_39)
		{
			T6ZM::PatchT6ZM_V39();
		}
		else if (Global::Game::Version == GAME_VERSION_41)
		{
			T6ZM::PatchT6ZM_V41();
		}
		else if (Global::Game::Version == GAME_VERSION_42)
		{
			T6ZM::PatchT6ZM_V42();
		}
		else
		{
			Auth_Error("Unsupported Zombies version");
			ExitProcess(0x8000D3AD);
		}

		return;
	}

	if (Global::Game::Type == GAME_TYPE_MP)
	{
		Global::Variables::Steam_AppID = 202990;
		OutputDebugStringA("[Patch] Multiplayer exe detected\n");

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
			T6MP::PatchT6MP_V44();
		}
		else
		{
			Auth_Error("Unsupported Multiplayer version");
			ExitProcess(0x8000D3AD);
		}

		return;
	}

	Auth_Error("Game type not supported. Aborting startup");
	ExitProcess(0x8000D3AD);
}
