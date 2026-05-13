#include "STDPatches.h"
#include "PatchFunctions.h"

void T6MP::PatchT6MP_V44()
{
	__try
	{
		MPLog(MP_COLOR_INFO, "BOOT", "PatchT6MP_V44 start");

		DebugConsole::Initialize();

		MPLog(MP_COLOR_INFO, "BOOT", "DebugConsole initialized");

		SteamCommon::LoadOverlay();

		MPLog(MP_COLOR_INFO, "BOOT", "Overlay initialized");

		DumpHandler::Initialize();

		MPLog(MP_COLOR_INFO, "BOOT", "DumpHandler initialized");

		DisableMP44OnlineServiceChecks();

		MPLog(MP_COLOR_INFO, "BOOT", "Online checks disabled");

		HANDLE frontendThread =
			CreateThread(
				NULL,
				0,
				MP44FrontendWaitThread,
				NULL,
				0,
				NULL);

		if (frontendThread)
		{
			CloseHandle(frontendThread);

			MPLog(MP_COLOR_INFO, "BOOT", "Frontend thread started");
		}

		MPLog(MP_COLOR_OK, "BOOT", "PatchT6MP_V44 complete");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MessageBoxA(
			NULL,
			"PatchT6MP_V44 crashed",
			"T6",
			MB_ICONERROR);
	}
}