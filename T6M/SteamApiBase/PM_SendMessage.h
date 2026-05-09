#pragma once

#include "xNP_Stubs.h"

extern "C" __declspec(dllexport) int __cdecl PM_SendMessage(
	PMID onlineID,
	uint8_t* message,
	int messageLength
);