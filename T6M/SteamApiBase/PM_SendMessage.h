#pragma once

#include "xNP_Stubs.h"

int __cdecl PM_SendMessage(
    PMID onlineID,
    uint8_t* message,
    int messageLength
);