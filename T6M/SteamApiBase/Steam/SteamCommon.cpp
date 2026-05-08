#include "..\STDInc.h"

HMODULE SteamCommon::SteamOverlay = 0;
char SteamCommon::SteamPath[MAX_PATH] = { 0 };

static SteamApps005* _SteamApps = new SteamApps005;
static SteamClient012* _SteamClient = new SteamClient012;
static SteamFriends013* _SteamFriends = new SteamFriends013;
static SteamContentServer002* _SteamContentServer = new SteamContentServer002;
static SteamGameServer011* _SteamGameServer = new SteamGameServer011;
static SteamHTTP001* _SteamHTTP = new SteamHTTP001;
static SteamMatchmaking009* _SteamMatchMaking = new SteamMatchmaking009;
static SteamMatchmakingServers002* _SteamMatchMakingServers = new SteamMatchmakingServers002;
static SteamNetworking005* _SteamNetworking = new SteamNetworking005;
static SteamRemoteStorage008* _SteamRemoteStorage = new SteamRemoteStorage008;
static SteamUser016* _SteamUser = new SteamUser016;
static SteamUserStats011* _SteamUserStats = new SteamUserStats011;
static SteamUtils005* _SteamUtils = new SteamUtils005;

bool ValueExists(char* Section, char* Key, char* File)
{
    char Buffer[512]{};
    return GetPrivateProfileStringA(Section, Key, NULL, Buffer, sizeof(Buffer), File) > 0;
}

void SteamCommon::Init()
{
    char Buffer[512]{};
    char Path[512] = ".\\" USERDATA_PATH "\\GameSettings.ini";

    if (GetPrivateProfileStringA("Settings", "Username", "Unknown", Buffer, sizeof(Buffer), Path) > 0)
        strcpy_s(Global::Variables::Steam_Username, Buffer);

    if (GetPrivateProfileStringA("Settings", "Password", "123456", Buffer, sizeof(Buffer), Path) > 0)
        strcpy_s(Global::Variables::Steam_Password, Buffer);

    if (GetPrivateProfileStringA("Settings", "Language", "english", Buffer, sizeof(Buffer), Path) > 0)
        strcpy_s(Global::Variables::Steam_Language, Buffer);

    xNP_Initiate();

    if (!xNP_Connect(MASTER_SERVER, 14030))
    {
        Addresses::Com_Error(ERR_FROM_STARTUP, "Could not connect to 'SONSORED' Server at " MASTER_SERVER);
        return;
    }

    xNPAuthenticateResult* result = nullptr;

    xNPAsync<xNPAuthenticateResult>* async =
        xNP_CreateSessionWithCredentials(
            Global::Variables::Steam_Username,
            Global::Variables::Steam_Password,
            Global::Variables::Steam_Username
        );

    result = async->Wait();

    if (!result)
    {
        Addresses::Com_Error(
            ERR_FROM_STARTUP,
            "Could not authenticate to 'SONSORED' at " MASTER_SERVER " -- result was null."
        );
        return;
    }

    if (result->result != 0)
    {
        switch (result->result)
        {
        case 1:
            Addresses::Com_Error(
                ERR_FROM_STARTUP,
                "Could not authenticate to 'SONSORED' at " MASTER_SERVER " -- bad details | wrong username and password."
            );
            break;

        case 4:
            Addresses::Com_Error(
                ERR_FROM_STARTUP,
                "Could not authenticate to 'SONSORED' at " MASTER_SERVER " -- already logged in."
            );
            break;

        case 2:
            Addresses::Com_Error(
                ERR_FROM_STARTUP,
                "Could not authenticate to 'SONSORED' at " MASTER_SERVER " -- service unavailable."
            );
            break;

        case 3:
            Addresses::Com_Error(
                ERR_FROM_STARTUP,
                "Could not authenticate to 'SONSORED' at " MASTER_SERVER " -- banned."
            );
            break;

        default:
            Addresses::Com_Error(
                ERR_FROM_STARTUP,
                "Could not authenticate to 'SONSORED' at " MASTER_SERVER " -- unknown error."
            );
            break;
        }

        return;
    }

    xNP_SetRichPresence("currentGame", "t6mp");
    xNP_SetRichPresenceBody("Playing T6MP");
    xNP_StoreRichPresence();

    xNPAsync<xNPGetOnlineFriensResult>* async2 = xNP_GetOnlinePlayers("pt6");

    if (async2 && async2->Wait())
        Global::Variables::Onlines = async2->Wait()->TotalOnlines;
    else
        Global::Variables::Onlines = 0;
}

void SteamCommon::SetSteamDirectory()
{
    HKEY hRegKey;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, STEAM_REGISTRY_PATH, 0, KEY_QUERY_VALUE, &hRegKey) == ERROR_SUCCESS)
    {
        DWORD dwLength = sizeof(SteamCommon::SteamPath);
        RegQueryValueExA(hRegKey, "InstallPath", NULL, NULL, (BYTE*)SteamCommon::SteamPath, &dwLength);
        RegCloseKey(hRegKey);

        SetDllDirectoryA(SteamCommon::SteamPath);
    }
}

void SteamCommon::LoadOverlay()
{
    if (!*SteamCommon::SteamPath)
        SteamCommon::SetSteamDirectory();

    SteamCommon::SteamOverlay = GetModuleHandleA(GAMEOVERLAY_LIB);

    if (!SteamCommon::SteamOverlay)
        SteamCommon::SteamOverlay = LoadLibraryA(GAMEOVERLAY_LIB);
}

SteamAPICall_t SteamCommon::RequestEncryptedAppTicket(const void* pUserData, int cbUserData)
{
    uint64 userID = GetSteamID()->ConvertToUint64();

    memset(encryptedAppTicket, 0, sizeof(encryptedAppTicket));
    memcpy(&encryptedAppTicket[32], pUserData, min(cbUserData, sizeof(encryptedAppTicket) - 32));
    memcpy(&encryptedAppTicket[cbUserData + 32], &userID, sizeof(userID));

    SteamAPICall_t callID = SteamCallback::RegisterCall();

    EncryptedAppTicketResponse_t* response =
        (EncryptedAppTicketResponse_t*)malloc(sizeof(EncryptedAppTicketResponse_t));

    if (response)
        response->m_eResult = k_EResultOK;

    return callID;
}

CSteamID* SteamCommon::GetSteamID()
{
    static CSteamID cachedSteamID;

    xNPID myid = 0;
    xNP_GetxNPID(&myid);

    cachedSteamID = CSteamID(myid);
    return &cachedSteamID;
}

AppId_t SteamCommon::GetSteamAppID()
{
    return Global::Variables::Steam_AppID;
}

char* SteamCommon::GetEncryptedAppTicket()
{
    return encryptedAppTicket;
}

char* SteamCommon::GetPlayerName()
{
    return Global::Variables::Steam_Username;
}

void SteamCommon::SetPlayerName(const char* newName)
{
    if (newName)
        strcpy_s(Global::Variables::Steam_Username, newName);
}

ISteamApps* SteamCommon::GetSteamApps()
{
    return (ISteamApps*)_SteamApps;
}

ISteamFriends* SteamCommon::GetSteamFriends()
{
    return (ISteamFriends*)_SteamFriends;
}

ISteamClient* SteamCommon::GetSteamClient()
{
    return (ISteamClient*)_SteamClient;
}

ISteamGameServer* SteamCommon::GetSteamGameServer()
{
    return (ISteamGameServer*)_SteamGameServer;
}

ISteamGameServerStats* SteamCommon::GetSteamGameServerStats()
{
    return NULL;
}

ISteamHTTP* SteamCommon::GetSteamHTTP()
{
    return (ISteamHTTP*)_SteamHTTP;
}

ISteamMatchmaking* SteamCommon::GetSteamMatchmaking()
{
    return (ISteamMatchmaking*)_SteamMatchMaking;
}

ISteamMatchmakingServers* SteamCommon::GetSteamMatchmakingServers()
{
    return (ISteamMatchmakingServers*)_SteamMatchMakingServers;
}

ISteamNetworking* SteamCommon::GetSteamNetworking()
{
    return (ISteamNetworking*)_SteamNetworking;
}

ISteamRemoteStorage* SteamCommon::GetSteamRemoteStorage()
{
    return (ISteamRemoteStorage*)_SteamRemoteStorage;
}

ISteamUser* SteamCommon::GetSteamUser()
{
    return (ISteamUser*)_SteamUser;
}

ISteamUserStats* SteamCommon::GetSteamUserStats()
{
    return (ISteamUserStats*)_SteamUserStats;
}

ISteamUtils* SteamCommon::GetSteamUtils()
{
    return (ISteamUtils*)_SteamUtils;
}

ISteamBilling* SteamCommon::GetSteamBilling()
{
    return NULL;
}

ISteamContentServer* SteamCommon::GetSteamContentServer()
{
    return (ISteamContentServer*)_SteamContentServer;
}

ISteamMasterServerUpdater* SteamCommon::GetSteamMasterServerUpdater()
{
    return NULL;
}

ISteamController* SteamCommon::GetSteamController()
{
    return NULL;
}

ISteamScreenshots* SteamCommon::GetSteamScreenshots()
{
    return NULL;
}

ISteamUnifiedMessages* SteamCommon::GetSteamUnifiedMessages()
{
    return NULL;
}

CSteamID* SteamCommon::steamID = NULL;
AppId_t SteamCommon::steamAppID = NULL;
char SteamCommon::encryptedAppTicket[128]{};