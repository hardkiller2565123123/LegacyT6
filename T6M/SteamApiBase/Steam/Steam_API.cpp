#include "..\STDInc.h"
#include <Windows.h>

#pragma warning(disable: 4530)
#pragma warning(disable: 4577)

static void SafeDebugLog(const char* text)
{
    if (text)
    {
        OutputDebugStringA(text);
        OutputDebugStringA("\n");
    }
}

static int g_DummySteamApps = 1;
static int g_DummySteamClient = 1;
static int g_DummySteamContentServer = 1;
static int g_DummySteamFriends = 1;
static int g_DummySteamGameServer = 1;
static int g_DummySteamMasterServerUpdater = 1;
static int g_DummySteamMatchmaking = 1;
static int g_DummySteamMatchmakingServers = 1;
static int g_DummySteamNetworking = 1;
static int g_DummySteamRemoteStorage = 1;
static int g_DummySteamUser = 1;
static int g_DummySteamUserStats = 1;
static int g_DummySteamUtils = 1;
static int g_DummySteamClientGameServerStorage = 1;

static void* SafeInterface(void* ptr, void* fallback, const char* name)
{
    if (ptr)
        return ptr;

    SafeDebugLog(name);
    return fallback;
}

extern "C"
{
    __declspec(dllexport) HSteamPipe __cdecl GetHSteamPipe()
    {
        return 1;
    }

    __declspec(dllexport) HSteamUser __cdecl GetHSteamUser()
    {
        return 1;
    }

    __declspec(dllexport) HSteamPipe __cdecl SteamAPI_GetHSteamPipe()
    {
        return 1;
    }

    __declspec(dllexport) HSteamUser __cdecl SteamAPI_GetHSteamUser()
    {
        return 1;
    }

    __declspec(dllexport) const char* __cdecl SteamAPI_GetSteamInstallPath()
    {
        static char buffer[MAX_PATH] = { 0 };

        DWORD len = GetCurrentDirectoryA(MAX_PATH, buffer);

        if (len == 0 || len >= MAX_PATH)
        {
            lstrcpyA(buffer, ".\\");
        }

        return buffer;
    }

    __declspec(dllexport) bool __cdecl SteamAPI_Init()
    {
        __try
        {
            SteamCommon::Init();
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamCommon::Init crashed");
        }

        return true;
    }

    __declspec(dllexport) bool __cdecl SteamAPI_InitSafe()
    {
        return SteamAPI_Init();
    }

    __declspec(dllexport) void __cdecl SteamAPI_RegisterCallResult(void* result, SteamAPICall_t callID)
    {
        if (!result)
            return;

        __try
        {
            SteamCallback::RegisterCallResult(callID, static_cast<CallbackBase*>(result));
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamAPI_RegisterCallResult crashed");
        }
    }

    __declspec(dllexport) void __cdecl SteamAPI_RegisterCallback(void* callback, int type)
    {
        if (!callback)
            return;

        __try
        {
            SteamCallback::RegisterCallback(static_cast<CallbackBase*>(callback), type);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamAPI_RegisterCallback crashed");
        }
    }

    __declspec(dllexport) void __cdecl SteamAPI_RunCallbacks()
    {
        __try
        {
            if (xNP_RunCallBacks)
            {
                xNP_RunCallBacks();
            }
            else
            {
                SafeDebugLog("[steam_api] xNP_RunCallBacks was NULL");
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] xNP_RunCallBacks crashed");
        }

        __try
        {
            SteamCallback::RunCallbacks();
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamCallback::RunCallbacks crashed");
        }
    }

    __declspec(dllexport) void __cdecl SteamAPI_SetMiniDumpComment(const char* msg)
    {
        UNREFERENCED_PARAMETER(msg);
    }

    __declspec(dllexport) bool __cdecl SteamAPI_SetTryCatchCallbacks(bool bTryCatchCallbacks)
    {
        UNREFERENCED_PARAMETER(bTryCatchCallbacks);
        return true;
    }

    __declspec(dllexport) void __cdecl SteamAPI_Shutdown()
    {
        __try
        {
            if (xNP_Shutdown)
            {
                xNP_Shutdown();
            }
            else
            {
                SafeDebugLog("[steam_api] xNP_Shutdown was NULL");
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] xNP_Shutdown crashed");
        }
    }

    __declspec(dllexport) void __cdecl SteamAPI_UnregisterCallResult(void* result, SteamAPICall_t callID)
    {
        if (!result)
            return;

        __try
        {
            SteamCallback::UnregisterCallResult(callID, static_cast<CallbackBase*>(result));
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamAPI_UnregisterCallResult crashed");
        }
    }

    __declspec(dllexport) void __cdecl SteamAPI_UnregisterCallback(void* callback, int type)
    {
        if (!callback)
            return;

        __try
        {
            SteamCallback::UnregisterCallback(static_cast<CallbackBase*>(callback), type);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamAPI_UnregisterCallback crashed");
        }
    }

    __declspec(dllexport) void __cdecl SteamAPI_WriteMiniDump(uint32 structuredExceptionCode, void* exceptionInfo, uint32 buildID)
    {
        UNREFERENCED_PARAMETER(structuredExceptionCode);
        UNREFERENCED_PARAMETER(exceptionInfo);
        UNREFERENCED_PARAMETER(buildID);
    }

    __declspec(dllexport) void* __cdecl SteamApps()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamApps(), &g_DummySteamApps, "[steam_api] SteamApps NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamApps crashed");
            return &g_DummySteamApps;
        }
    }

    __declspec(dllexport) void* __cdecl SteamClient()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamClient(), &g_DummySteamClient, "[steam_api] SteamClient NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamClient crashed");
            return &g_DummySteamClient;
        }
    }

    __declspec(dllexport) void* __cdecl SteamContentServer()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamContentServer(), &g_DummySteamContentServer, "[steam_api] SteamContentServer NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamContentServer crashed");
            return &g_DummySteamContentServer;
        }
    }

    __declspec(dllexport) void* __cdecl SteamContentServerUtils()
    {
        return &g_DummySteamUtils;
    }

    __declspec(dllexport) bool __cdecl SteamContentServer_Init(unsigned int localIP, unsigned short port)
    {
        UNREFERENCED_PARAMETER(localIP);
        UNREFERENCED_PARAMETER(port);
        return true;
    }

    __declspec(dllexport) void __cdecl SteamContentServer_RunCallbacks()
    {
    }

    __declspec(dllexport) void __cdecl SteamContentServer_Shutdown()
    {
    }

    __declspec(dllexport) void* __cdecl SteamFriends()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamFriends(), &g_DummySteamFriends, "[steam_api] SteamFriends NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamFriends crashed");
            return &g_DummySteamFriends;
        }
    }

    __declspec(dllexport) void* __cdecl SteamGameServer()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamGameServer(), &g_DummySteamGameServer, "[steam_api] SteamGameServer NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamGameServer crashed");
            return &g_DummySteamGameServer;
        }
    }

    __declspec(dllexport) void* __cdecl SteamGameServerUtils()
    {
        return &g_DummySteamUtils;
    }

    __declspec(dllexport) bool __cdecl SteamGameServer_BSecure()
    {
        return true;
    }

    __declspec(dllexport) HSteamPipe __cdecl SteamGameServer_GetHSteamPipe()
    {
        return 1;
    }

    __declspec(dllexport) HSteamUser __cdecl SteamGameServer_GetHSteamUser()
    {
        return 1;
    }

    __declspec(dllexport) int32 __cdecl SteamGameServer_GetIPCCallCount()
    {
        return 0;
    }

    __declspec(dllexport) uint64 __cdecl SteamGameServer_GetSteamID()
    {
        return 76561197960287930ULL;
    }

    __declspec(dllexport) bool __cdecl SteamGameServer_Init(
        uint32 ip,
        uint16 port,
        uint16 gamePort,
        EServerMode serverMode,
        int gameAppID,
        const char* gameDir,
        const char* versionString)
    {
        UNREFERENCED_PARAMETER(ip);
        UNREFERENCED_PARAMETER(port);
        UNREFERENCED_PARAMETER(gamePort);
        UNREFERENCED_PARAMETER(serverMode);
        UNREFERENCED_PARAMETER(gameAppID);
        UNREFERENCED_PARAMETER(gameDir);
        UNREFERENCED_PARAMETER(versionString);

        return true;
    }

    __declspec(dllexport) bool __cdecl SteamGameServer_InitSafe(
        uint32 ip,
        uint16 port,
        uint16 gamePort,
        EServerMode serverMode,
        int gameAppID,
        const char* gameDir,
        const char* versionString,
        unsigned long dongs)
    {
        UNREFERENCED_PARAMETER(dongs);

        return SteamGameServer_Init(
            ip,
            port,
            gamePort,
            serverMode,
            gameAppID,
            gameDir,
            versionString);
    }

    __declspec(dllexport) void __cdecl SteamGameServer_RunCallbacks()
    {
    }

    __declspec(dllexport) void __cdecl SteamGameServer_Shutdown()
    {
    }

    __declspec(dllexport) void* __cdecl SteamMasterServerUpdater()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamMasterServerUpdater(), &g_DummySteamMasterServerUpdater, "[steam_api] SteamMasterServerUpdater NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamMasterServerUpdater crashed");
            return &g_DummySteamMasterServerUpdater;
        }
    }

    __declspec(dllexport) void* __cdecl SteamMatchmaking()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamMatchmaking(), &g_DummySteamMatchmaking, "[steam_api] SteamMatchmaking NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamMatchmaking crashed");
            return &g_DummySteamMatchmaking;
        }
    }

    __declspec(dllexport) void* __cdecl SteamMatchmakingServers()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamMatchmakingServers(), &g_DummySteamMatchmakingServers, "[steam_api] SteamMatchmakingServers NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamMatchmakingServers crashed");
            return &g_DummySteamMatchmakingServers;
        }
    }

    __declspec(dllexport) void* __cdecl SteamNetworking()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamNetworking(), &g_DummySteamNetworking, "[steam_api] SteamNetworking NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamNetworking crashed");
            return &g_DummySteamNetworking;
        }
    }

    __declspec(dllexport) void* __cdecl SteamRemoteStorage()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamRemoteStorage(), &g_DummySteamRemoteStorage, "[steam_api] SteamRemoteStorage NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamRemoteStorage crashed");
            return &g_DummySteamRemoteStorage;
        }
    }

    __declspec(dllexport) void* __cdecl SteamUser()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamUser(), &g_DummySteamUser, "[steam_api] SteamUser NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamUser crashed");
            return &g_DummySteamUser;
        }
    }

    __declspec(dllexport) void* __cdecl SteamUserStats()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamUserStats(), &g_DummySteamUserStats, "[steam_api] SteamUserStats NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamUserStats crashed");
            return &g_DummySteamUserStats;
        }
    }

    __declspec(dllexport) void* __cdecl SteamUtils()
    {
        __try
        {
            return SafeInterface(SteamCommon::GetSteamUtils(), &g_DummySteamUtils, "[steam_api] SteamUtils NULL");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            SafeDebugLog("[steam_api] SteamUtils crashed");
            return &g_DummySteamUtils;
        }
    }

    __declspec(dllexport) HSteamUser __cdecl Steam_GetHSteamUserCurrent()
    {
        return 1;
    }

    __declspec(dllexport) void __cdecl Steam_RegisterInterfaceFuncs(void* module)
    {
        UNREFERENCED_PARAMETER(module);
    }

    __declspec(dllexport) void __cdecl Steam_RunCallbacks(HSteamPipe hSteamPipe, bool bGameServerCallbacks)
    {
        UNREFERENCED_PARAMETER(hSteamPipe);
        UNREFERENCED_PARAMETER(bGameServerCallbacks);

        SteamAPI_RunCallbacks();
    }

    __declspec(dllexport) bool __cdecl SteamAPI_RestartApp(int appid)
    {
        UNREFERENCED_PARAMETER(appid);
        return true;
    }

    __declspec(dllexport) bool __cdecl SteamAPI_RestartAppIfNecessary(uint32 appID)
    {
        Global::Variables::Steam_AppID = appID;
        return false;
    }

    __declspec(dllexport) bool __cdecl SteamAPI_IsSteamRunning()
    {
        return true;
    }

    __declspec(dllexport) void* g_pSteamClientGameServer = &g_DummySteamClientGameServerStorage;
}