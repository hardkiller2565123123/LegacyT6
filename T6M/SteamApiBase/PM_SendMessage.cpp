#include "PM_SendMessage.h"
#include <Windows.h>

extern "C" __declspec(dllexport) int __cdecl PM_SendMessage(
	PMID onlineID,
	uint8_t* message,
	int messageLength
)
{
	OutputDebugStringA("[steam_api.dll] PM_SendMessage called\n");

	return xNP_SendMessage(
		onlineID,
		message,
		messageLength);
}