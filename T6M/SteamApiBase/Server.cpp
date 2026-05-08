#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "STDInc.h"
#include <Windows.h>

typedef void(__cdecl* ui_keyboard_new_t)(int Type);
ui_keyboard_new_t ui_keyboard_new = (ui_keyboard_new_t)0x475CE0;

bool Server::LoggedIn = false;

static bool g_ServerNetworkPatched = false;
static bool g_ServerThreadStarted = false;
static bool g_ServerCriticalSectionsReady = false;

static void ServerDebugLog(const char* text)
{
    if (text)
    {
        OutputDebugStringA(text);
        OutputDebugStringA("\n");
    }
}

static bool IsWritableAddress(DWORD address)
{
    if (address == 0)
        return false;

    MEMORY_BASIC_INFORMATION mbi = { 0 };

    if (!VirtualQuery((LPCVOID)address, &mbi, sizeof(mbi)))
        return false;

    if (mbi.State != MEM_COMMIT)
        return false;

    if (mbi.Protect & PAGE_GUARD)
        return false;

    if (mbi.Protect & PAGE_NOACCESS)
        return false;

    return true;
}

static bool WritePointerSafe(DWORD targetAddress, DWORD replacement, const char* name)
{
    if (!IsWritableAddress(targetAddress))
    {
        ServerDebugLog(name);
        return false;
    }

    __try
    {
        DWORD oldProtect = 0;

        if (VirtualProtect((LPVOID)targetAddress, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            *(DWORD*)targetAddress = replacement;

            DWORD temp = 0;
            VirtualProtect((LPVOID)targetAddress, sizeof(DWORD), oldProtect, &temp);

            return true;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        ServerDebugLog("[Server] WritePointerSafe crashed");
    }

    return false;
}

static void PatchNetworkPointers()
{
    bool ok = true;

    ok &= WritePointerSafe(
        Addresses::custom_gethostbyname,
        (DWORD)dw_Entry::custom_gethostbyname,
        "[Server] custom_gethostbyname address invalid");

    ok &= WritePointerSafe(
        Addresses::dw_recvfrom,
        (DWORD)dw_Entry::dw_recvfrom,
        "[Server] dw_recvfrom address invalid");

    ok &= WritePointerSafe(
        Addresses::dw_sendto,
        (DWORD)dw_Entry::dw_sendto,
        "[Server] dw_sendto address invalid");

    ok &= WritePointerSafe(
        Addresses::dw_recv,
        (DWORD)dw_Entry::dw_recv,
        "[Server] dw_recv address invalid");

    ok &= WritePointerSafe(
        Addresses::dw_send,
        (DWORD)dw_Entry::dw_send,
        "[Server] dw_send address invalid");

    ok &= WritePointerSafe(
        Addresses::dw_connect,
        (DWORD)dw_Entry::dw_connect,
        "[Server] dw_connect address invalid");

    ok &= WritePointerSafe(
        Addresses::dw_select,
        (DWORD)dw_Entry::dw_select,
        "[Server] dw_select address invalid");

    g_ServerNetworkPatched = ok;

    if (ok)
        ServerDebugLog("[Server] Network pointers patched");
    else
        ServerDebugLog("[Server] Some network pointers failed to patch");
}

static void StartDwSystem()
{
    if (!g_ServerCriticalSectionsReady)
    {
        InitializeCriticalSection(&dw_Handler::incomingCS);
        InitializeCriticalSection(&dw_Handler::packetCS);

        g_ServerCriticalSectionsReady = true;
        ServerDebugLog("[Server] Critical sections initialized");
    }

    if (!g_ServerThreadStarted)
    {
        HANDLE threadHandle = CreateThread(
            NULL,
            0,
            dw_Handler::dw_thread,
            NULL,
            0,
            NULL);

        if (threadHandle)
        {
            CloseHandle(threadHandle);
            g_ServerThreadStarted = true;
            ServerDebugLog("[Server] DW thread started");
        }
        else
        {
            ServerDebugLog("[Server] Failed to start DW thread");
        }
    }

    if (xNP_RegisterMessageCallback)
    {
        __try
        {
            xNP_RegisterMessageCallback(dw_Handler::dw_im_received);
            ServerDebugLog("[Server] xNP message callback registered");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            ServerDebugLog("[Server] xNP_RegisterMessageCallback crashed");
        }
    }
    else
    {
        ServerDebugLog("[Server] xNP_RegisterMessageCallback is NULL");
    }
}

void Server::SetupLogin()
{
    if (Server::LoggedIn)
    {
        PatchNetworkPointers();
        StartDwSystem();

        __try
        {
            ((void(*)(int))0x45CA30)(0);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            ServerDebugLog("[Server] 0x45CA30 call crashed");
        }
    }
    else
    {
        if (Addresses::Cbuf_AddText)
        {
            __try
            {
                Addresses::Cbuf_AddText(0, "ui_keyboard_new 2");
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                ServerDebugLog("[Server] Cbuf_AddText crashed");
            }
        }
        else
        {
            ServerDebugLog("[Server] Cbuf_AddText is NULL");
        }
    }
}

__declspec(naked) void Server::ConnectingHook_Stub()
{
    __asm
    {
        push 0
        call Server::SetupLogin
        add esp, 0x04
        retn
    }
}

void Server::Initialize()
{
    ServerDebugLog("[Server] Initialize started");

    PatchNetworkPointers();
    StartDwSystem();

    ServerDebugLog("[Server] Initialize finished");
}