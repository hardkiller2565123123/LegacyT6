#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "STDInc.h"

SOCKET dw_Entry::dwSocket = INVALID_SOCKET;
ULONG dw_Entry::masterAddr = 0;

static const char* LOCAL_MASTER_IP = "127.0.0.1";

static void DWPrint(const char* text)
{
	if (!text)
		return;

	printf("%s\n", text);
	OutputDebugStringA(text);
	OutputDebugStringA("\n");
}

unsigned int dw_Entry::oneAtATimeHash(char* key)
{
	unsigned int hash = 0;

	if (!key)
		return hash;

	size_t len = strlen(key);

	for (size_t i = 0; i < len; ++i)
	{
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

static bool IsDemonwareHost(const char* name)
{
	if (!name)
		return false;

	return
		_stricmp(name, "ops2-pc-auth.prod.demonware.net") == 0 ||
		_stricmp(name, "ops2-pc-lobby.prod.demonware.net") == 0 ||
		_stricmp(name, "cod7-stun.us.demonware.net") == 0 ||
		_stricmp(name, "cod7-stun.eu.demonware.net") == 0 ||
		_stricmp(name, "cod7-stun.jp.demonware.net") == 0 ||
		_stricmp(name, "cod7-stun.au.demonware.net") == 0;
}

hostent* WINAPI dw_Entry::custom_gethostbyname(const char* name)
{
	printf("[DW] gethostbyname: %s\n", name ? name : "NULL");
	Log::Debug("custom_gethostbyname", name ? name : "NULL");

	if (IsDemonwareHost(name))
	{
		DWPrint("[DW] redirecting Demonware host to localhost");
		Log::Debug("custom_gethostbyname", "redirecting Demonware host to localhost");

		return gethostbyname(LOCAL_MASTER_IP);
	}

	return gethostbyname(name);
}

static ULONG GetLocalMasterAddress()
{
	ULONG addr = inet_addr(LOCAL_MASTER_IP);

	if (addr == INADDR_NONE)
		return 0;

	return addr;
}

int WINAPI dw_Entry::dw_connect(SOCKET socket, const sockaddr* name, int namelen)
{
	DWPrint("[DW] connect called");

	if (!masterAddr)
		masterAddr = GetLocalMasterAddress();

	if (name && namelen == sizeof(sockaddr_in))
	{
		const sockaddr_in* addr = (const sockaddr_in*)name;

		printf(
			"[DW] connect ip=%s port=%d\n",
			inet_ntoa(addr->sin_addr),
			ntohs(addr->sin_port));

		if (addr->sin_addr.S_un.S_addr == masterAddr && addr->sin_port == htons(3074))
		{
			DWPrint("[DW] captured DW socket");
			Log::Debug("dw_connect", "captured DW socket");

			dw_Entry::dwSocket = socket;
			return 0;
		}
	}

	return connect(socket, name, namelen);
}

int WINAPI dw_Entry::dw_send(SOCKET socket, const char* buf, int len, int flags)
{
	if (socket == dw_Entry::dwSocket)
	{
		printf("[DW] send intercepted len=%d\n", len);
		dw_Handler::dw_handle_packet(buf, len);
		return len;
	}

	return send(socket, buf, len, flags);
}

int WINAPI dw_Entry::dw_sendto(SOCKET socket, const char* buf, int len, int flags, const sockaddr* to, int tolen)
{
	if (socket == dw_Entry::dwSocket)
	{
		printf("[DW] sendto intercepted len=%d\n", len);
		dw_Handler::dw_handle_packet(buf, len);
		return len;
	}

	return sendto(socket, buf, len, flags, to, tolen);
}

int WINAPI dw_Entry::dw_recv(SOCKET socket, char* buf, int len, int flags)
{
	if (socket != dw_Entry::dwSocket)
		return recv(socket, buf, len, flags);

	if (dw_Handler::dw_packet_available())
	{
		int result = dw_Handler::dw_dequeue_packet(buf, len);
		printf("[DW] recv dequeued len=%d\n", result);
		return result;
	}

	WSASetLastError(WSAEWOULDBLOCK);
	return SOCKET_ERROR;
}

int WINAPI dw_Entry::dw_recvfrom(SOCKET socket, char* buf, int len, int flags, sockaddr* from, int* fromlen)
{
	if (socket != dw_Entry::dwSocket)
		return recvfrom(socket, buf, len, flags, from, fromlen);

	if (dw_Handler::dw_packet_available())
	{
		int result = dw_Handler::dw_dequeue_packet(buf, len);
		printf("[DW] recvfrom dequeued len=%d\n", result);
		return result;
	}

	WSASetLastError(WSAEWOULDBLOCK);
	return SOCKET_ERROR;
}

int WINAPI dw_Entry::dw_select(
	int nfds,
	fd_set* readfds,
	fd_set* writefds,
	fd_set* exceptfds,
	const timeval* timeout)
{
	bool hadDWSocketWrite = false;
	bool hadDWSocketRead = false;
	bool hadDWSocketExcept = false;

	if (dw_Entry::dwSocket != INVALID_SOCKET)
	{
		if (writefds && FD_ISSET(dw_Entry::dwSocket, writefds))
		{
			hadDWSocketWrite = true;
			FD_CLR(dw_Entry::dwSocket, writefds);
		}

		if (readfds && FD_ISSET(dw_Entry::dwSocket, readfds))
		{
			hadDWSocketRead = true;
			FD_CLR(dw_Entry::dwSocket, readfds);
		}

		if (exceptfds && FD_ISSET(dw_Entry::dwSocket, exceptfds))
		{
			hadDWSocketExcept = true;
			FD_CLR(dw_Entry::dwSocket, exceptfds);
		}
	}

	int retval = select(nfds, readfds, writefds, exceptfds, timeout);

	if (retval == SOCKET_ERROR)
		retval = 0;

	if (dw_Entry::dwSocket != INVALID_SOCKET)
	{
		if (hadDWSocketWrite && writefds)
		{
			FD_SET(dw_Entry::dwSocket, writefds);
			retval++;
		}

		if (hadDWSocketRead && readfds && dw_Handler::dw_packet_available())
		{
			FD_SET(dw_Entry::dwSocket, readfds);
			retval++;
		}
	}

	return retval;
}