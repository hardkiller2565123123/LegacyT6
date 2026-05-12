// ==========================================================
// General includes.
// ==========================================================

#pragma once

#define DEVBUILD

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef DEVBUILD
#define HHSDBG() Log::Debug("SteamApiBase", "%s()", __FUNCTION__)
#else
#define HHSDBG()
#endif

// ==========================================================
// Runtime Includes
// ==========================================================

#pragma region Runtime

//#include <Windows.h>

#include <string>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <list>
#include <map>
#include <queue>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <chrono>
#include <thread>
#include <direct.h>

#pragma endregion

// ==========================================================
// Networking
// ==========================================================

#include <ws2tcpip.h>

// ==========================================================
// Steam
// ==========================================================

#include "Steam\SteamCallback.h"
#include "Steam\Steam.h"

// ==========================================================
// Utils
// ==========================================================

#include "Utils\Log.h"

// ==========================================================
// Core
// ==========================================================

#include "T6.h"
#include "DumpHandler.h"
#include "Hooking.h"
#include "Patch.h"
#include "FileIO.h"
#include "hString.h"
#include "Addresses.h"
#include "PatchT6MP.h"
#include "PatchT6ZM.h"
#include "Global.h"
#include "Patterns.h"
#include "Scriptability.h"
#include "Notify.h"
#include "Bots.h"
#include "Command.h"
#include "HudElem.h"
#include "MainPatchFunctions.h"
#include "ColoredName.h"
#include "InGameConsole.h"
#include "ServerList.h"
#include "Zone.h"
#include "Experimental.h"
#include "Brandings.h"
#include "DLC.h"
#include "Popup.h"
#include "GamePlay.h"


// ==========================================================
// Models / Structs
// ==========================================================

#include "XModel.h"

// ==========================================================
// NP / Matchmaking / Server
// IMPORTANT:
// libnp.h MUST come before xNP_Stubs.h
// ==========================================================

#include "libnp.h"
#include "xNP_Stubs.h"
#include "PM_SendMessage.h"

#include "Crypto.h"
#include "Entry.h"
#include "Auth.h"
#include "BitBuffer.h"
#include "ByteBuffer.h"
#include "dwMessage.h"
#include "TitleUtils.h"
#include "Storage.h"
#include "dw_Handler.h"
#include "bdProfile.h"

#include "bdMatchMakingInfo.h"
#include "dw_match.h"

// ==========================================================
// Server
// ==========================================================

#include "Server.h"