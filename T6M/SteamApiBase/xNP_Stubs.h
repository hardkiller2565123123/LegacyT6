#pragma once

#include <stdint.h>
#include <cstring>

typedef unsigned long long PMID;
typedef unsigned long long xNPID;
typedef unsigned long long PMSID;

struct xNPAuthenticateResult
{
	int result = 0;
	xNPID id = 0x110000100000001ULL;
	uint8_t sessionToken[32]{};
};

struct xNPGetOnlineFriensResult
{
	int TotalOnlines = 0;
};

struct PMSessionInfo
{
	int players = 0;
};

struct PMWriteUserFileResult
{
	int result = 0;
};

struct PMGetUserFileResult
{
	int result = 0;
	int fileSize = 0;
	uint8_t* buffer = nullptr;
};

struct PMGetPublisherFileResult
{
	int result = 0;
	int fileSize = 0;
	uint8_t* buffer = nullptr;
};

struct PMGetPT5ProfileDataEntry
{
	xNPID xNPID = 0;
	char* data = nullptr;
};

struct PMGetPT5ProfileDataResult
{
	int numResults = 0;
	PMGetPT5ProfileDataEntry results[1024]{};
};

struct PMPT5ProfileData {};

template <typename T>
class PMAsync
{
	T result_{};

public:
	T* Wait()
	{
		return &result_;
	}

	T* GetResult()
	{
		return &result_;
	}
};

template <typename T>
class xNPAsync
{
	T result_{};

public:
	T* Wait()
	{
		return &result_;
	}

	T* GetResult()
	{
		return &result_;
	}
};

static inline int xNP_CopyFallbackFile(
	const char* filename,
	uint8_t* buffer,
	int bufferSize,
	const char* fallbackData)
{
	if (!buffer || bufferSize <= 0 || !fallbackData)
		return 0;

	int fallbackSize = (int)strlen(fallbackData);

	if (fallbackSize <= 0 || fallbackSize > bufferSize)
		return 0;

	memcpy(buffer, fallbackData, fallbackSize);

	return fallbackSize;
}

static inline const char* xNP_GetPublisherFallbackData(const char* filename)
{
	if (!filename)
		return "T6M_PLACEHOLDER_DATA";

	if (strstr(filename, "online_tu17_mp"))
		return "T6M_ONLINE_TU17_MP_PLACEHOLDER";

	if (strstr(filename, "ffotd_tu17_mp"))
		return "T6M_FFOTD_TU17_MP_PLACEHOLDER";

	if (strstr(filename, "online_tu17_zm"))
		return "T6M_ONLINE_TU17_ZM_PLACEHOLDER";

	if (strstr(filename, "ffotd_tu17_zm"))
		return "T6M_FFOTD_TU17_ZM_PLACEHOLDER";

	if (strstr(filename, "lsssk0"))
		return "T6M_LSSSK0_PLACEHOLDER";

	if (strstr(filename, "lsssk1"))
		return "T6M_LSSSK1_PLACEHOLDER";

	if (strstr(filename, "largeheatmap.raw"))
		return "T6M_LARGE_HEATMAP_PLACEHOLDER_DATA";

	return "T6M_PLACEHOLDER_DATA";
}

static inline const char* xNP_GetUserFallbackData(const char* filename)
{
	if (!filename)
		return "{}";

	if (strstr(filename, "metplayer"))
		return "{}";

	if (strstr(filename, "badmpdata"))
		return "{}";

	return "{}";
}

inline void xNP_Initiate()
{
}

inline bool xNP_Connect(const char*, int)
{
	return true;
}

inline void xNP_Shutdown()
{
}

inline void xNP_RunCallBacks()
{
}

inline xNPAsync<xNPAuthenticateResult>* xNP_CreateSessionWithCredentials(
	const char*,
	const char*,
	const char*)
{
	static xNPAsync<xNPAuthenticateResult> async;

	async.GetResult()->result = 0;
	async.GetResult()->id = 0x110000100000001ULL;

	memset(
		async.GetResult()->sessionToken,
		0,
		sizeof(async.GetResult()->sessionToken));

	return &async;
}

inline void xNP_SetRichPresence(const char*, const char*)
{
}

inline void xNP_SetRichPresenceBody(const char*)
{
}

inline void xNP_StoreRichPresence()
{
}

inline const char* xNP_GetFriendRichPresence(xNPID)
{
	return "Playing T6MP";
}

inline const char* xNP_GetFriendRichPresence(xNPID, const char* key)
{
	if (!key)
		return "";

	if (strcmp(key, "currentGame") == 0)
		return "t6mp";

	if (strcmp(key, "status") == 0)
		return "Playing T6MP";

	return "";
}

inline int xNP_GetTotalFriends()
{
	return 0;
}

inline xNPID xNP_GetFriendGUID(int)
{
	return 0;
}

inline const char* xNP_GetFriendlyName(xNPID)
{
	return "Player";
}

inline int xNP_GetFriendPresence(xNPID)
{
	return 0;
}

inline xNPAsync<xNPGetOnlineFriensResult>* xNP_GetOnlinePlayers(const char*)
{
	static xNPAsync<xNPGetOnlineFriensResult> async;

	async.GetResult()->TotalOnlines = 0;

	return &async;
}

inline void xNP_GetxNPID(xNPID* id)
{
	if (id)
		*id = 0x110000100000001ULL;
}

inline int xNP_SendMessage(PMID, uint8_t*, int)
{
	return 1;
}

inline void xNP_RegisterMessageCallback(
	void(__cdecl*)(PMID, const uint8_t*, uint32_t))
{
}

inline PMAsync<PMWriteUserFileResult>* xNP_WriteUserFile(
	const char*,
	xNPID,
	uint8_t*,
	int)
{
	static PMAsync<PMWriteUserFileResult> async;

	async.GetResult()->result = 0;

	return &async;
}

inline PMAsync<PMGetUserFileResult>* xNP_GetUserFile(
	const char* filename,
	xNPID)
{
	static PMAsync<PMGetUserFileResult> async;

	async.GetResult()->buffer = nullptr;
	async.GetResult()->fileSize = 0;
	async.GetResult()->result = 1;

	return &async;
}

inline PMAsync<PMGetUserFileResult>* xNP_GetUserFile(
	const char*,
	xNPID,
	uint8_t* buffer,
	int bufferSize
)
{
	static PMAsync<PMGetUserFileResult> async;

	const char* data = "{}";
	int size = (int)strlen(data);

	if (buffer && bufferSize >= size)
	{
		memcpy(buffer, data, size);
		async.GetResult()->buffer = buffer;
		async.GetResult()->fileSize = size;
		async.GetResult()->result = 0;
	}

	return &async;
}

inline PMAsync<PMGetPublisherFileResult>* xNP_GetPublisherFile(
	const char* filename)
{
	static PMAsync<PMGetPublisherFileResult> async;

	async.GetResult()->buffer = nullptr;
	async.GetResult()->fileSize = 0;
	async.GetResult()->result = 1;

	return &async;
}

inline PMAsync<PMGetPublisherFileResult>* xNP_GetPublisherFile(
	const char*,
	uint8_t* buffer,
	int bufferSize
)
{
	static PMAsync<PMGetPublisherFileResult> async;

	const char* data = "T6M";
	int size = (int)strlen(data);

	if (buffer && bufferSize >= size)
	{
		memcpy(buffer, data, size);
		async.GetResult()->buffer = buffer;
		async.GetResult()->fileSize = size;
		async.GetResult()->result = 0;
	}

	return &async;
}

inline PMAsync<PMGetPT5ProfileDataResult>* xNP_GetPT5ProfileData(
	int,
	xNPID*,
	PMPT5ProfileData*)
{
	static PMAsync<PMGetPT5ProfileDataResult> async;

	async.GetResult()->numResults = 0;

	return &async;
}