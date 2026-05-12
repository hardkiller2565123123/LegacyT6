#include "STDInc.h"
#include "PM_SendMessage.h"

int __cdecl PM_SendMessage(
	PMID onlineID,
	uint8_t* message,
	int messageLength)
{
	UNREFERENCED_PARAMETER(onlineID);
	UNREFERENCED_PARAMETER(message);
	UNREFERENCED_PARAMETER(messageLength);

	OutputDebugStringA("[steam_api.dll] PM_SendMessage stub called\n");

	return 1;
}