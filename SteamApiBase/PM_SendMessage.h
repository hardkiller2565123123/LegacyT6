#pragma once

#include <stdint.h>

typedef unsigned long long PMID;

int __cdecl PM_SendMessage(
    PMID onlineID,
    uint8_t* message,
    int messageLength
);