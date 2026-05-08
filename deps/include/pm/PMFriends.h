// ==========================================================
// PlusMW3 project
// 
// Component: xPM
// Sub-component: libPM
// Purpose: definitions for the PM friends service
//
// Initial author: NTAuthority
// Started: 2012-01-16
// ==========================================================

enum EPresenceState
{
	PresenceStateUnknown,
	PresenceStateOnline,
	PresenceStateAway,
	PresenceStateExtendedAway,
	PresenceStateOffline
};


struct PMGetOnlineFriensResult
{
	int TotalOnlines;
};


struct PMGetUserAvatarResult
{
	EGetFileResult result;
	int32_t guid;
	uint32_t fileSize;
	uint8_t* buffer;
};

struct PMPT5ProfileData
{
	PMID PMID;
	char* data;
};

struct PMPIW5ProfileData
{
	PMID PMID;
	char* data;
};

struct PMProfileData
{
	PMID PMID;
	int32_t experience;
	int32_t prestige;
};

struct PMGetProfileDataResult
{
	int32_t numResults;
	PMProfileData* results;
};

struct PMGetPIW5ProfileDataResult
{
	int32_t numResults;
	PMPIW5ProfileData* results;
};

struct PMGetPT5ProfileDataResult
{
	int32_t numResults;
	PMPT5ProfileData* results;
};