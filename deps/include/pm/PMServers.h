// ==========================================================
// PlusMW3 project
// 
// Component: xPM
// Sub-component: libPM
// Purpose: definitions for the PM server list service
//
// Initial author: NTAuthority
// Started: 2012-05-10
// ==========================================================

enum EServersResult
{
	ServersResultOK = 0,
	ServersResultNotAllowed = 1,
	ServersResultNotFound = 2
};

class PMSessionInfo
{
public:
	uint32_t address;
	int32_t port;
	PMID PMid;
	char* hostname;
	char* mapname;
	int32_t players;
	int32_t maxplayers;
	int32_t version;
	char* type;
};

class PMPIW5SessionInfo
{
public:
	uint32_t InternalIPAdress;
	int32_t InternalPort;
	char* Address;
	char* SessionID;
	uint32_t unknown1;
	uint32_t unknown2;
	int32_t User;
	uint32_t IPAdress;
	int32_t Port;
	int32_t Unknown3;
	int32_t Playlist;
	int32_t PlaylistVersion;
	int32_t Unknown4;
	int32_t Unknown5;
	int32_t Unknown6;
	int32_t Unknown7;
	int32_t Unknown8;
	int32_t Unknown9;
	int32_t UnknownA;
	int32_t UnknownB;
	char* UnknownBlob;
	int32_t UnknownC;
	uint32_t UnknownUInt1;
	uint32_t UnknownUInt2;
	float UnknownFloat1;
	float UnknownFloat2;
	int32_t UnknownD;
	int32_t UnknownE;
	int32_t UnknownF;
	int32_t UnknownG;
	int32_t UnknownH;
	int32_t UnknownI;
	int32_t UnknownJ;
	int32_t UnknownK;
	int32_t UnknownL;
	int32_t UnknownM;
	int32_t UnknownN;
	int32_t UnknownO;
	int32_t UnknownP;
	int32_t UnknownQ;
	int32_t UnknownR;
	int32_t UnknownS;
	PMID UnknownUInt64;
	int32_t UnknownInt1;
	uint32_t Players;
};

class PMLobbyData
{
public:
	char* dw_sec_key;
	char* dw_sec_kid;
	char* dw_serialized_addr;
	char* lobbyid;
	PMID pmid;
};

class PMPT5SessionInfo
{
public:
	char* address;
	int32_t port;
	PMID PMid;
	char* hostname;
	char* mapname;
	int32_t players;
	int32_t maxplayers;
	int32_t version;
	char* sessionid;
	uint32_t unknown1;
	uint32_t unknown2;
	int32_t NetcodeVersion;
	char* unknownblobl2;
	PMID unknownlong1;
	int32_t intgametype;
	int32_t playlist;
	int32_t unknown6;
	int32_t unknown7;
	int32_t unknown8;
	uint32_t licensetype;
	int32_t unknownA;
	int32_t unknownB;
	int32_t unknownD;
	PMID onlineId;
	int32_t wager;
	int32_t unknownF;
	char* unknownstring3;
	PMID unknownlong3;
	uint32_t stateddl;
	char* type;
};

typedef uint64_t PMSID;

class PMCreateSessionResult
{
public:
	EServersResult result;
	PMSID sid;
};
