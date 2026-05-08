// ==========================================================
// PlusMW3 project
// 
// Component: xPM
// Sub-component: libPM
// Purpose: definitions for the PM authentication service
//
// Initial author: NTAuthority
// Started: 2011-06-28
// ==========================================================

enum EAuthenticateResult
{
	AuthenticateResultOK = 0,
	AuthenticateResultBadDetails = 1,
	AuthenticateResultServiceUnavailable = 2,
	AuthenticateResultBanned = 3,
	AuthenticateResultAlreadyLoggedIn = 4,
	AuthenticateResultUnknown = 9999
};

enum EExternalAuthState
{
	ExternalAuthStatePassed = 0,		// owns the game, cleared for playing
	ExternalAuthStateUnverified = 1,	// unverified account, needs further verification
	ExternalAuthStatePirate = 2,		// yarrrrrrrr, the user's a pirate!
	ExternalAuthStateError = 3,			// an error occurred during authentication; pass on as usual
	ExternalAuthStatePrivate = 4		// the external authentication profile is set as 'private'.
};

class PMRegisterServerResult
{
public:
	EAuthenticateResult result;
	char licenseKey[32];
	uint32_t serverID;
};

class PMAuthenticateResult
{
public:
	EAuthenticateResult result;
	PMID id;
	uint8_t sessionToken[32];
};

enum E_ValidateTicketResult
{
	ValidateTicketResultOK = 0,
	ValidateTicketResultInvalid = 1
};

class PMValidateTicketResult
{
public:
	E_ValidateTicketResult result;
	PMID id;
	int32_t groupID;
};

#pragma pack(push, 1)
class PMAuthenticateTicket
{
public:
	int32_t version;
	PMID clientID;
	PMID serverID;
	uint32_t time;
};
#pragma pack(pop)