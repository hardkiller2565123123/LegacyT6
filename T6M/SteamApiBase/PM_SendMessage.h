#pragma once

#include <stdint.h>

typedef unsigned long long PMID;

#ifdef __cplusplus
extern "C"
{
#endif

	__declspec(dllexport) int __cdecl PM_SendMessage(
		PMID onlineID,
		uint8_t* message,
		int messageLength);

#ifdef __cplusplus
}
#endif