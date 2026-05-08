// ==========================================================
// PlusMW3 project
// 
// Component: xPM
// Sub-component: libPM
// Purpose: external include file
//
// Initial author: NTAuthority
// updated by : Sa3id
// Started: 2011-06-28
// ==========================================================

#pragma once

#include "PMTypeDefs.h"
#include "PMAsync.h"
#include "PMAuthenticate.h"
#include "PMStorage.h"
#include "PMFriends.h"
#include "PMServers.h"

// ----------------------------------------------------------
// Initialization/shutdown functions
// ----------------------------------------------------------

// starts up the network platform functions
LIBPM_API bool LIBPM_CALL PM_Initiate();

// cleans up and shuts down the network platform
LIBPM_API bool LIBPM_CALL PM_Shutdown();

// connects to a PM server
LIBPM_API bool LIBPM_CALL PM_Connect(const char* server, uint16_t port);

// log output callback
typedef void (LIBPM_CALL * PMLogCB)(const char* message);

// sets the output callback for log messages
LIBPM_API void LIBPM_CALL PM_SetLogCallback(PMLogCB callback);

// load game module axaxax
LIBPM_API void LIBPM_CALL PM_LoadGameModule(char* str, int version);

// ----------------------------------------------------------
// Callback handling
// ----------------------------------------------------------

// handles and dispatches callbacks
LIBPM_API bool LIBPM_CALL PM_RunCallBacks();

// ----------------------------------------------------------
// Authentication service
// ----------------------------------------------------------

// authenticates using an external auth token
LIBPM_API PMAsync<PMAuthenticateResult>* LIBPM_CALL PM_CreateSessionWithKey(const char* authToken);

// authenticates using a username/password
LIBPM_API PMAsync<PMAuthenticateResult>* LIBPM_CALL PM_CreateSessionWithCredentials(const char* username, const char* password, const char* hwid);

// authenticates using a license key
LIBPM_API PMAsync<PMAuthenticateResult>* LIBPM_CALL PM_CreateSessionWithLicense(const char* licenseKey);

// registers a game server license key
LIBPM_API PMAsync<PMRegisterServerResult>* LIBPM_CALL PM_RegisterGameServer(const char* configPath);

// validates a user ticket
LIBPM_API PMAsync<PMValidateTicketResult>* LIBPM_CALL PM_ValidateTicket(const void* ticket, size_t ticketSize, uint32_t clientIP, PMID clientID);

// obtains a user ticket for server authentication
LIBPM_API bool LIBPM_CALL PM_RequestTicket(void* buffer, size_t bufferSize, PMID targetServer);

// gets the PMID for the current client. returns false (and does not change the output buffer) if not yet authenticated
LIBPM_API bool LIBPM_CALL PM_GetPMID(PMID* pID);

// gets the user group for the current client, returns 0 if not authenticated
LIBPM_API int LIBPM_CALL PM_GetUserGroup();

// function to register a callback to kick a client by PMID
LIBPM_API void LIBPM_CALL PM_RegisterKickCallback(void(__cdecl * callback)(PMID, const char*));

// function to register a callback when external authentication status changes
LIBPM_API void LIBPM_CALL PM_RegisterExternAuthCallback(void(__cdecl * callback)(EExternalAuthState));

// sends this messages when PlusCI loaded
LIBPM_API void LIBPM_CALL PM_SendStatusCode(int state);

// loads the game DLL for the specified version number
//LIBPM_API void* LIBPM_CALL LoadT5Module(int version);

// Uploads Screenshot
LIBPM_API PMAsync<PMGVUploadResultMessage>* LIBPM_CALL PMGVUpload(const uint8_t* buffer, size_t bufferLength);

// obtains user's overwatch status
LIBPM_API PMAsync<PMGSResult>* LIBPM_CALL PM_GetGS(PMID pmid);

// ----------------------------------------------------------
// Storage service
// ----------------------------------------------------------

// obtains a file from the remote global per-title storage
LIBPM_API PMAsync<PMGetPublisherFileResult>* LIBPM_CALL PM_GetPublisherFile(const char* fileName, uint8_t* buffer, size_t bufferLength);

// obtains a file from the remote per-user storage
LIBPM_API PMAsync<PMGetUserFileResult>* LIBPM_CALL PM_GetUserFile(const char* fileName, PMID PMID, uint8_t* buffer, size_t bufferLength);

// uploads a file to the remote per-user storage
LIBPM_API PMAsync<PMWriteUserFileResult>* LIBPM_CALL PM_WriteUserFile(const char* fileName, PMID PMID, const uint8_t* buffer, size_t bufferLength);

// sends a random string to the PM server
LIBPM_API void LIBPM_CALL PMSendRandomString(const char* str);

// ----------------------------------------------------------
// Friends service
// ----------------------------------------------------------

// Gets total count of online players ingame
LIBPM_API PMAsync<PMGetOnlineFriensResult>* LIBPM_CALL PM_GetOnlinePlayers(char* tags);

// sets the external platform ID for Steam
LIBPM_API void LIBPM_CALL PM_SetFriendSteamGUID(uint64_t steamID);

// obtains profile data for a list of PMIDs
LIBPM_API PMAsync<PMGetProfileDataResult>* LIBPM_CALL PM_GetProfileData(uint32_t numIDs, const PMID* PMIDs, PMProfileData* outData);

LIBPM_API PMAsync<PMGetPIW5ProfileDataResult>* LIBPM_CALL PM_GetPIW5ProfileData(uint32_t numIDs, const PMID* PMIDs, PMPIW5ProfileData* outData);

LIBPM_API PMAsync<PMGetPT5ProfileDataResult>* LIBPM_CALL PM_GetPT5ProfileData(uint32_t numIDs, const PMID* PMIDs, PMPT5ProfileData* outData);

// returns if the friends API is available
LIBPM_API bool LIBPM_CALL PM_IsFriendsSystemAvailable();

// gets the number of registered friends
LIBPM_API uint32_t LIBPM_CALL PM_GetTotalFriends();

// gets a specific friend's PMID - index is from 0...[PM_GetNumFriends() - 1]
LIBPM_API PMID LIBPM_CALL PM_GetFriendGUID(int32_t index);

// gets the name for a friend
// will only work with PMIDs known to the client - currently only if they're friends
LIBPM_API const char* LIBPM_CALL PM_GetFriendlyName(PMID PMID);

// gets the presence state for a friend
LIBPM_API EPresenceState LIBPM_CALL PM_GetFriendPresence(PMID PMID);

// sets a presence key/value pair
// a value of NULL removes the key, if existent
LIBPM_API void LIBPM_CALL PM_SetRichPresence(const char* key, const char* value);

// sets the presence body
LIBPM_API void LIBPM_CALL PM_SetRichPresenceBody(const char* body);

// uploads the rich presence data
LIBPM_API void LIBPM_CALL PM_StoreRichPresence();

// gets a rich presence value for a friend
// will only work with friends, not other known PMIDs
LIBPM_API const char* LIBPM_CALL PM_GetFriendRichPresence(PMID PMID, const char* key);

// gets the rich presence body for a friend
LIBPM_API const char* LIBPM_CALL PM_GetFriendRichPresenceBody(PMID PMID);

// gets an avatar for any client
LIBPM_API PMAsync<PMGetUserAvatarResult>* LIBPM_CALL PM_GetUserAvatar(PMID id, uint8_t* buffer, size_t bufferLength);

// ----------------------------------------------------------
// Server list service
// ----------------------------------------------------------



// creates a remote session
LIBPM_API PMAsync<PMCreateSessionResult>* LIBPM_CALL PM_CreateSession(PMSessionInfo* data);

LIBPM_API PMAsync<PMCreateSessionResult>* LIBPM_CALL PM_CreatePIW5Session(PMPIW5SessionInfo* data);

LIBPM_API PMAsync<PMCreateSessionResult>* LIBPM_CALL PM_CreatePT5Session(PMPT5SessionInfo* data);

LIBPM_API PMAsync<EServersResult>* LIBPM_CALL PM_UpdatePT5Session(PMSID sid, PMPT5SessionInfo* data);

LIBPM_API PMAsync<EServersResult>* LIBPM_CALL PM_UpdatePT5SessionPlayers(PMSID sid, int32_t curPlayers);

LIBPM_API PMAsync<EServersResult>* LIBPM_CALL PM_UpdatePIW5SessionPlayers(PMSID sid, int32_t curPlayers);

LIBPM_API PMAsync<EServersResult>* LIBPM_CALL PM_UpdatePIW5Session(PMSID sid, PMPIW5SessionInfo* data);
// updates a session
LIBPM_API PMAsync<EServersResult>* LIBPM_CALL PM_UpdateSession(PMSID sid, const char* hostname, const char* mapname, int16_t curPlayers, int16_t maxPlayers);

// deletes a session
LIBPM_API PMAsync<EServersResult>* LIBPM_CALL PM_DeleteSession(PMSID sid);

// refreshes the session list - tags are separated by single spaces
LIBPM_API PMAsync<bool>* LIBPM_CALL PM_RefreshSessions(const char* tags);

LIBPM_API PMAsync<bool>* LIBPM_CALL PM_RefreshPIW5Sessions(const char* tags);

LIBPM_API PMAsync<bool>* LIBPM_CALL PM_RefreshPT5Sessions(const char* tags);


// gets the number of sessions
LIBPM_API int32_t LIBPM_CALL PM_GetNumSessions();

LIBPM_API int32_t LIBPM_CALL PM_GetNumPT5Sessions();

LIBPM_API int32_t LIBPM_CALL PM_GetNumPIW5Sessions();

// gets a single session's info
LIBPM_API void LIBPM_CALL PM_GetSessionData(int32_t index, PMSessionInfo* out);

LIBPM_API void LIBPM_CALL PM_GetPIW5SessionData(int32_t index, PMPIW5SessionInfo* out);

LIBPM_API void LIBPM_CALL PM_GetPT5SessionData(int32_t index, PMPT5SessionInfo* out);

// ----------------------------------------------------------
// Messaging service
// ----------------------------------------------------------

// sends arbitrary data to a client

LIBPM_API void LIBPM_CALL PM_IW5SendMessage(char* message);

LIBPM_API void LIBPM_CALL PM_SendMessage(PMID PMid, const uint8_t* data, uint32_t length);

LIBPM_API void LIBPM_CALL PM_PT5SendMessage(PMID PMid, const uint8_t* blob, uint32_t length, const uint8_t* attachment, uint32_t length2, char* name);

// function to register a callback when a message is received
// arguments: source PMID, data, length

LIBPM_API void LIBPM_CALL PM_RegisterPubNewsCallback(void(__cdecl * callback)(const char*));

LIBPM_API void LIBPM_CALL PM_RegisterMessageCallback(void(__cdecl * callback)(PMID, const uint8_t*, uint32_t));

LIBPM_API void LIBPM_CALL PM_RegisterPT5MessageCallback(void(__cdecl * callback)(PMID, const uint8_t*, uint32_t, const uint8_t*, uint32_t, char*));

LIBPM_API void LIBPM_CALL PM_RegisterScreenshotCallback(void(__cdecl * callback)(PMID pmid));