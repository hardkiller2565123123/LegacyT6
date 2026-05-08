#include "STDInc.h"

Hook::Stomp ServerList::GetServerNameFromListHook;
Hook::Stomp ServerList::GetServerCountHook;

int ServerList::ServerCount = 0;

static bool IsValidPtr(DWORD address)
{
    return address != 0;
}

static void DebugLog(const char* text)
{
    if (text)
    {
        OutputDebugStringA(text);
        OutputDebugStringA("\n");
    }
}

static bool IsZm()
{
    return Global::Game::Type == GameType::GAME_TYPE_ZM;
}

DWORD ServerList::GetServerNameFromList(int Server)
{
    if (!IsValidPtr(Addresses::ServerBaseAddress))
    {
        DebugLog("[ServerList] ServerBaseAddress is NULL in GetServerNameFromList");
        return 0;
    }

    if (Server < 0 || Server >= ServerList::ServerCount)
    {
        DebugLog("[ServerList] Invalid server index in GetServerNameFromList");
        return 0;
    }

    DWORD serverEntry = Addresses::ServerBaseAddress + (0x110 * Server);
    DWORD nameAddress = serverEntry + 0x88;

    __try
    {
        return *(DWORD*)nameAddress;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DebugLog("[ServerList] Failed reading server name pointer");
        return 0;
    }
}

int ServerList::GetServerCount()
{
    return ServerList::ServerCount;
}

void ServerList::WriteServerInformation(
    DWORD Server,
    const char* ServerName,
    const char* MapName,
    bool isInGame,
    int com_maxclients,
    int clients,
    int ping,
    int minping,
    int maxping,
    const char* gamemode)
{
    if (!IsValidPtr(Server))
    {
        DebugLog("[ServerList] WriteServerInformation got NULL server address");
        return;
    }

    if (!ServerName)
        ServerName = "Unknown Server";

    if (!MapName)
        MapName = IsZm() ? "tomb_zm" : "mp_nuketown_2020";

    if (!gamemode)
        gamemode = IsZm() ? "ZMUI_ZCLASSIC" : "tdm";

    __try
    {
        ZeroMemory((void*)(Server + 0x88), 0x15);
        ZeroMemory((void*)(Server + 0xA8), 0x15);
        ZeroMemory((void*)(Server + 0xE0), 0x15);

        memcpy((void*)(Server + 0x88), ServerName, min(strlen(ServerName), size_t(0x14)));
        memcpy((void*)(Server + 0xA8), MapName, min(strlen(MapName), size_t(0x14)));
        memcpy((void*)(Server + 0xE0), gamemode, min(strlen(gamemode), size_t(0x14)));

        *(bool*)(Server + 0xF0) = isInGame;

        *(BYTE*)(Server + 0x72) = (BYTE)com_maxclients;
        *(BYTE*)(Server + 0x71) = (BYTE)clients;

        *(WORD*)(Server + 0x7C) = (WORD)minping;
        *(WORD*)(Server + 0x7E) = (WORD)maxping;
        *(WORD*)(Server + 0x80) = (WORD)ping;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DebugLog("[ServerList] WriteServerInformation crashed while writing memory");
    }
}

void ServerList::AddServer()
{
    if (!IsValidPtr(Addresses::ServerBaseAddress))
    {
        DebugLog("[ServerList] ServerBaseAddress is NULL in AddServer");
        ServerList::ServerCount = 0;
        return;
    }

    ServerList::ServerCount = 25;

    for (int i = 0; i < ServerList::ServerCount; i++)
    {
        DWORD serverAddress = Addresses::ServerBaseAddress + (0x110 * i);

        const char* mapName = IsZm() ? "tomb_zm" : "mp_nuketown_2020";
        const char* gameMode = IsZm() ? "ZMUI_ZCLASSIC" : "tdm";

        ServerList::WriteServerInformation(
            serverAddress,
            hString::va("Test Server %i", i),
            mapName,
            (i % 2) == 0,
            18,
            1,
            80,
            0,
            140,
            gameMode);
    }
}

void ServerList::Initialize()
{
    DebugLog("[ServerList] Initialize started");

    ServerList::ServerCount = 0;

    if (Addresses::Dvar_RegisterBool)
    {
        __try
        {
            Addresses::Dvar_RegisterBool("ui_netSource", false, 0, "");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            DebugLog("[ServerList] Dvar_RegisterBool crashed");
        }
    }
    else
    {
        DebugLog("[ServerList] Dvar_RegisterBool is NULL");
    }

    if (!IsValidPtr(Addresses::GetServerNameFromList))
    {
        DebugLog("[ServerList] GetServerNameFromList address is NULL, skipping hook");
    }
    else
    {
        __try
        {
            ServerList::GetServerNameFromListHook.Initialize(
                Addresses::GetServerNameFromList,
                ServerList::GetServerNameFromList);

            ServerList::GetServerNameFromListHook.InstallHook();

            DebugLog("[ServerList] GetServerNameFromList hook installed");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            DebugLog("[ServerList] GetServerNameFromList hook crashed");
        }
    }

    if (!IsValidPtr(Addresses::GetServerCount))
    {
        DebugLog("[ServerList] GetServerCount address is NULL, skipping hook");
    }
    else
    {
        __try
        {
            ServerList::GetServerCountHook.Initialize(
                Addresses::GetServerCount,
                ServerList::GetServerCount);

            ServerList::GetServerCountHook.InstallHook();

            DebugLog("[ServerList] GetServerCount hook installed");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            DebugLog("[ServerList] GetServerCount hook crashed");
        }
    }

    ServerList::AddServer();

    DebugLog("[ServerList] Initialize finished");
}